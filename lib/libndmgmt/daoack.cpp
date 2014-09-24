/*
 * Copyright (C) 2009-2013 Michael Richardson <mcr@sandelman.ca>
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

void network_interface::receive_daoack(struct in6_addr from,
                                       struct in6_addr ip6_to,
                                       const  time_t now,
                                       const u_char *dat, const int daoack_len)
{
    unsigned int dat_len = daoack_len;
    debug->info("  processing daoack(%u)",daoack_len);

    struct nd_rpl_daoack *daoack = (struct nd_rpl_daoack *)dat;
    unsigned char *dat2 = (unsigned char *)(daoack+1);
    dat_len -= sizeof(struct nd_rpl_daoack);

    if(this->packet_too_short("daoack", daoack_len, sizeof(*daoack))) return;

    dagid_t dagid;
    char dagid_str[16*6];
    dagid[0]=0;
    if(RPL_DAOACK_D(daoack->rpl_flags)) {
        memcpy(&dagid, dat2, DAGID_LEN);
	dag_network::format_dagid(dagid_str, dat2);
        dat2 += DAGID_LEN;
	dat_len -= DAGID_LEN;
    }

    debug->info_more(" [instance:%u,daoseq:%u,dagid:%s]\n",
                daoack->rpl_instanceid,
                daoack->rpl_daoseq,
                dagid_str[0] ? dagid_str : "<elided>");

    /* XXX if rpl_instanceid is 0, then we are using a default DAG? */
    if(!RPL_DAOACK_D(daoack->rpl_flags)) {
	dag_network::globalStats[PS_DAOACK_NO_DAGID_IGNORED]++;
        return;
    }

    /* find the relevant DAG */
    class dag_network *dn = dag_network::find_by_dagid(dagid);

    if(dn) {
	/* and process it */
	dn->receive_daoack(this, from, ip6_to, now, daoack, dat2, dat_len);
    } else {
	dag_network::globalStats[PS_DAOACK_PACKET_IGNORED]++;
    }
}

int dag_network::build_daoack(unsigned char *buff,
                              unsigned int buff_len,
                              unsigned short seq_num)
{
    struct sockaddr_in6 addr;
    struct in6_addr *dest = NULL;
    struct icmp6_hdr  *icmp6;
    struct nd_rpl_daoack *daoack;
    unsigned char *nextopt;
    int optlen;
    int len = 0;

    memset(buff, 0, buff_len);

    icmp6 = (struct icmp6_hdr *)buff;
    icmp6->icmp6_type = ND_RPL_MESSAGE;
    icmp6->icmp6_code = ND_RPL_DAO_ACK;
    icmp6->icmp6_cksum = 0;

    daoack = (struct nd_rpl_daoack *)icmp6->icmp6_data8;
    nextopt = (unsigned char *)(daoack+1);

    daoack->rpl_instanceid = mInstanceid;
    daoack->rpl_flags = 0;
    daoack->rpl_flags |= RPL_DAOACK_D_MASK;

    daoack->rpl_daoseq     = seq_num;
    daoack->rpl_status     = 0; /* means child accepted */

    /* insert dagid, advance */
    {
        unsigned char *dagid = (unsigned char *)&daoack[1];
        memcpy(dagid, mDagid, 16);
        nextopt = dagid + 16;
    }

    /* recalculate length */
    len = ((caddr_t)nextopt - (caddr_t)buff);

    len = (len + 7)&(~0x7);  /* round up to next multiple of 64-bits */
    return len;
}


void network_interface::send_daoack(rpl_node &child, dag_network &dag, unsigned short seq_num)
{
    unsigned char icmp_body[2048];

    debug->log("sending DAOACK on if: %s%s\n",
               this->if_name,
	       this->faked() ? "(faked)" : "");
    memset(icmp_body, 0, sizeof(icmp_body));

    unsigned int icmp_len = dag.build_daoack(icmp_body, sizeof(icmp_body), seq_num);

    struct in6_addr dest = child.node_number();
    send_raw_icmp(&dest, icmp_body, icmp_len);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */
