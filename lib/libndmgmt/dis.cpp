/*
 * Copyright (C) 2016 Michael Richardson <mcr@sandelman.ca>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <time.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include "oswlibs.h"
#include "rpl.h"

}

#include "iface.h"
#include "dag.h"
#include "dis.h"


void network_interface::receive_dis(struct in6_addr from,
                                    struct in6_addr ip6_to,
                                    const  time_t now,
                                    const u_char *dat, const int dis_len)
{
    struct nd_rpl_dis *dis = (struct nd_rpl_dis *)dat;
    unsigned int dis_left = dis_len;

    if(this->packet_too_short("dis", dis_len, sizeof(*dis))) return;

    dis_left -= sizeof(*dis);

    /* we need to process the options to find the instanceid */
    rpl_dis decoded_dis(dis->rpl_dis_options,dis_left,dag_network::globalStats);

    struct rpl_dis_solicitedinfo *rplsi;
    while((rplsi = decoded_dis.rplsolicitedinfo()) != NULL) {
        class dag_network *dn = NULL;

        if((rplsi->rpl_dis_flags & (RPL_DIS_SI_I|RPL_DIS_SI_D))==(RPL_DIS_SI_I|RPL_DIS_SI_D)) {
            dn = dag_network::find_by_instanceid(rplsi->rpl_dis_instanceid,
                                                 rplsi->rpl_dis_dagid);
        }else if(rplsi->rpl_dis_flags & (RPL_DIS_SI_I)) {
            dn = dag_network::find_by_instanceid(rplsi->rpl_dis_instanceid);
        } else {
            /* just reset trickle timer? */
        }

        if(dn) {
            /* and process it */
            dn->receive_dis(this, from, ip6_to, now, dis, dis_len);
        } else {
            dag_network::globalStats[PS_DIS_PACKET_IGNORED]++;
        }
    }
}


void network_interface::send_dis(dag_network *dag)
{
    unsigned char icmp_body[2048];

    debug->log("sending DIS on if: %s for instanceID: %u\n",
               this->if_name, dag->get_instanceid());
    memset(icmp_body, 0, sizeof(icmp_body));

    int icmp_len = dag->build_dis(icmp_body, sizeof(icmp_body));

    if(icmp_len > 0) {
	/* NULL indicates use multicast */
	this->send_raw_icmp(NULL, icmp_body, icmp_len);
    }
}

/*
 * RFC6550, 6.7.9 says:
 *
 *        0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |   Type = 0x07 |Opt Length = 19| RPLInstanceID |V|I|D|  Flags  |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                                                               |
 *     +                                                               +
 *     |                                                               |
 *     +                            DODAGID                            +
 *     |                                                               |
 *     +                                                               +
 *     |                                                               |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |Version Number |
 *     +-+-+-+-+-+-+-+-+
 *
 *   V: The 'V' flag is the Version predicate.  The Version predicate is
 *       true if the receiver's DODAGVersionNumber matches the requested
 *       Version Number.  If the 'V' flag is cleared, then the Version
 *       field is not valid and the Version field MUST be set to zero on
 *       transmission and ignored upon receipt.
 *
 *   I: The 'I' flag is the InstanceID predicate.  The InstanceID
 *       predicate is true when the RPL node's current RPLInstanceID
 *       matches the requested RPLInstanceID.  If the 'I' flag is
 *       cleared, then the RPLInstanceID field is not valid and the
 *       RPLInstanceID field MUST be set to zero on transmission and
 *       ignored upon receipt.
 *
 *   D: The 'D' flag is the DODAGID predicate.  The DODAGID predicate is
 *       true if the RPL node's parent set has the same DODAGID as the
 *       DODAGID field.  If the 'D' flag is cleared, then the DODAGID
 *       field is not valid and the DODAGID field MUST be set to zero on
 *       transmission and ignored upon receipt.
 */
int dag_network::build_info_disopt(void)
{
    memset(optbuff, 0, sizeof(optbuff));
    struct rpl_dis_solicitedinfo *dissi = (struct rpl_dis_solicitedinfo *)optbuff;

    /*
     * we are always interested in the a specific instanceID, and
     * we don't care about the Version or DODAGID, so those bits are
     * not set.
     */
    dissi->rpl_dis_instanceid = this->mInstanceid;
    dissi->rpl_dis_flags = RPL_DIS_SI_I;

    this->optlen = sizeof(struct rpl_dis_solicitedinfo);

    return this->optlen;
}

int dag_network::build_dis(unsigned char *buff,
			   unsigned int buff_len)
{
    struct sockaddr_in6 addr;
    struct in6_addr   *dest = NULL;
    struct icmp6_hdr  *icmp6;
    struct nd_rpl_dis *dis;
    unsigned char *nextopt;
    int optlen;
    int len = 0;

    memset(buff, 0, buff_len);

    icmp6 = (struct icmp6_hdr *)buff;
    icmp6->icmp6_type = ND_RPL_MESSAGE;
    icmp6->icmp6_code = ND_RPL_DAG_IS;
    icmp6->icmp6_cksum = 0;

    dis = (struct nd_rpl_dis *)icmp6->icmp6_data8;

    nextopt = (unsigned char *)&dis[1];

    int disoptlen = build_info_disopt();
    int nextoptlen = 0;

    len = ((caddr_t)nextopt - (caddr_t)buff);
    nextoptlen = append_suboption(nextopt, disoptlen, RPL_DIS_SOLICITEDINFO);

    if(nextoptlen < 0) {
        /* failed to build DIO prefix option */
        return -1;
    }
    nextopt += nextoptlen;

    /* recalculate length */
    len = ((caddr_t)nextopt - (caddr_t)buff);

    len = (len + 7)&(~0x7);  /* round up to next multiple of 64-bits */
    return len;
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
