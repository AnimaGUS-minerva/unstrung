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
#include "dao.h"

void network_interface::receive_dao(struct in6_addr from,
                                    struct in6_addr ip6_to,
                                    const  time_t now,
                                    const u_char *dat, const int dao_len)
{
    unsigned int dat_len = dao_len;
    debug->info("  processing dao(%u)",dao_len);

    struct nd_rpl_dao *dao = (struct nd_rpl_dao *)dat;
    unsigned char *dat2 = (unsigned char *)(dao+1);
    dat_len -= sizeof(struct nd_rpl_dao);

    if(this->packet_too_short("dao", dao_len, sizeof(*dao))) return;

    dagid_t dagid;
    char dagid_str[16*6];
    dagid[0]=0;
    if(RPL_DAO_D(dao->rpl_flags)) {
        memcpy(&dagid, dat2, DAGID_LEN);
	dag_network::format_dagid(dagid_str, dat2);
        dat2 += DAGID_LEN;
	dat_len -= DAGID_LEN;
    }

    debug->info_more(" [instance:%u,daoseq:%u,%sdagid:%s]\n",
                dao->rpl_instanceid,
                dao->rpl_daoseq,
                RPL_DAO_K(dao->rpl_flags) ? "dao-ack," : "",
                dagid_str[0] ? dagid_str : "<elided>");

    /* XXX if rpl_instanceid is 0, then we are using a default DAG? */

    /* find the relevant DAG */
    /* if watching, then we find_or_make, which returns us an
     * inactive DAO.  This might be used to watch a network for RPL
     * activity.
     */
    class dag_network *dn;
    if(watching) {
	dn = dag_network::find_or_make_by_dagid(dagid,
						this->debug,
						watching);
    } else {
	dn = dag_network::find_by_dagid(dagid);
    }

    if(dn) {
	/* and process it */
	dn->receive_dao(this, from, ip6_to, now, dao, dat2, dat_len);
    } else {
	dag_network::globalStats[PS_DAO_PACKET_IGNORED]++;
    }
}

int dag_network::build_target_opt(struct in6_addr addr, int maskbits)
{
    memset(optbuff, 0, sizeof(optbuff));
    struct rpl_dao_target *daotg = (struct rpl_dao_target *)optbuff;
    maskbits = 128; // SV this needs to send the full prefix
    //if(maskbits > 128) maskbits = 128;    // for sanity.
    daotg->rpl_dao_flags     = 0x00;
    daotg->rpl_dao_prefixlen = maskbits;
    for(int i=0; i < (maskbits+7)/8; i++) {
        daotg->rpl_dao_prefix[i]=addr.s6_addr[i];
    }

    this->optlen = ((maskbits+7)/8 + 4);

    return this->optlen;
}

int dag_network::build_target_opt(ip_subnet prefix)
{
    return build_target_opt(prefix.addr.u.v6.sin6_addr, prefix.maskbits);
}



int dag_network::build_dao(unsigned char *buff,
			   unsigned int buff_len)
{
    struct sockaddr_in6 addr;
    struct in6_addr *dest = NULL;
    struct icmp6_hdr  *icmp6;
    struct nd_rpl_dao *dao;
    unsigned char *nextopt;
    int optlen;
    int len = 0;

    memset(buff, 0, buff_len);

    icmp6 = (struct icmp6_hdr *)buff;
    icmp6->icmp6_type = ND_RPL_MESSAGE;
    icmp6->icmp6_code = ND_RPL_DAO;
    icmp6->icmp6_cksum = 0;

    dao = (struct nd_rpl_dao *)icmp6->icmp6_data8;
    nextopt = (unsigned char *)(dao+1);

    dao->rpl_instanceid = mInstanceid;
    dao->rpl_flags = 0;
    dao->rpl_flags |= RPL_DAO_D_MASK|RPL_DAO_K_MASK;

    dao->rpl_daoseq     = mSequence;

    /* insert dagid, advance */
    {
        unsigned char *dagid = (unsigned char *)&dao[1];
        memcpy(dagid, mDagid, 16);
        nextopt = dagid + 16;
    }

    /* iterate through both maps; in theory, we can send multiple DAO? ZZZ */
    prefix_map_iterator pi = dag_children.begin();
    prefix_map_iterator endpi1 = dag_children.end();
    while(pi != endpi1) {
	prefix_node &pm = pi->second;

	/* add RPL_TARGET  */
	build_target_opt(pm.get_prefix());

	int nextoptlen;
	len = ((caddr_t)nextopt - (caddr_t)buff);
	nextoptlen = append_suboption(nextopt, buff_len-len, RPL_DAO_RPLTARGET);
	nextopt += nextoptlen;

	/* advance to next prefix */
	pi++;
    }
    pi = dag_prefixes.begin();
    prefix_map_iterator endpi2 = dag_prefixes.end();
    while(pi != endpi2) {
	prefix_node &pm = pi->second;

	/* add RPL_TARGET  */
	build_target_opt(pm.get_prefix());

	int nextoptlen;
	len = ((caddr_t)nextopt - (caddr_t)buff);
	nextoptlen = append_suboption(nextopt, buff_len-len, RPL_DAO_RPLTARGET);
	nextopt += nextoptlen;

	/* advance to next prefix */
	pi++;
    }

    /* add RPL_TRANSIT */
    /* add RPL_TARGET DESCRIPTION */

    /* recalculate length */
    len = ((caddr_t)nextopt - (caddr_t)buff);

    len = (len + 7)&(~0x7);  /* round up to next multiple of 64-bits */
    return len;
}


void network_interface::send_dao(rpl_node &parent, dag_network &dag)
{
    unsigned char icmp_body[2048];

    debug->log("sending DAO on if: %s%s\n",
               this->if_name,
	       this->faked() ? "(faked)" : "");
    memset(icmp_body, 0, sizeof(icmp_body));

    unsigned int icmp_len = dag.build_dao(icmp_body, sizeof(icmp_body));

    struct in6_addr dest = parent.node_number();
    send_raw_icmp(&dest, icmp_body, icmp_len);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */
