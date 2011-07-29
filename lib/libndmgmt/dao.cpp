/*
 * Copyright (C) 2009-2011 Michael Richardson <mcr@sandelman.ca>
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

void network_interface::receive_dao(const u_char *dao, const int dao_len)
{
    if(VERBOSE(this))
        fprintf(this->verbose_file, " processing dao(%u)\n",dao_len);
        
}


int network_interface::build_target_opt(ip_subnet prefix)
{
    memset(optbuff, 0, sizeof(optbuff));
    struct rpl_dao_target *daotg = (struct rpl_dao_target *)optbuff;

    daotg->rpl_dao_flags     = 0x00;
    daotg->rpl_dao_prefixlen = prefix.maskbits;
    for(int i=0; i < (prefix.maskbits+7)/8; i++) {
        daotg->rpl_dao_prefix[i]=prefix.addr.u.v6.sin6_addr.s6_addr[i];
    }

    this->optlen = ((prefix.maskbits+7)/8 + 1 + 4 + 4);

    return this->optlen;
}


int network_interface::build_dao(unsigned char *buff,
                                 unsigned int buff_len,
                                 ip_subnet prefix)
{
    uint8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
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
    icmp6->icmp6_code = ND_RPL_DAG_IO;
    icmp6->icmp6_cksum = 0;
    
    dao = (struct nd_rpl_dao *)icmp6->icmp6_data8;
    
    dao->rpl_instanceid = this->rpl_instanceid;
    dao->rpl_flags = 0;
    dao->rpl_flags |= RPL_DAO_D_MASK;
    
    dao->rpl_daoseq     = this->rpl_sequence;
    
    {
        unsigned char *dagid = (unsigned char *)&dao[1];
        memcpy(dagid, this->rpl_dagid, 16);
        nextopt = dagid + 16;
    }

    /* add RPL_TARGET  */
    build_target_opt(prefix);

    int nextoptlen;
    len = ((caddr_t)nextopt - (caddr_t)buff);
    nextoptlen = append_suboption(nextopt, buff_len-len, RPL_DAO_RPLTARGET);
    nextopt += nextoptlen;

    /* add RPL_TRANSIT */
    /* add RPL_TARGET DESCRIPTION */
    
    /* recalculate length */
    len = ((caddr_t)nextopt - (caddr_t)buff);

    len = (len + 7)&(~0x7);  /* round up to next multiple of 64-bits */
    return len;
}


void network_interface::send_dao(rpl_node &parent, prefix_node &pre)
{
    unsigned char icmp_body[2048];

    /* this is wrong, it needs to send the entire set of prefixes */
    debug->log("sending DAO on if: %s for prefix: %s\n",
               this->if_name, parent.node_name());
    memset(icmp_body, 0, sizeof(icmp_body));
    
    unsigned int icmp_len = build_dao(icmp_body, sizeof(icmp_body),
                                      pre.get_prefix());

    struct in6_addr dest = parent.node_number();
    send_raw_icmp(&dest, icmp_body, icmp_len);
}




/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
