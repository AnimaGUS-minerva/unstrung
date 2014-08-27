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
#include "dio.h"

void network_interface::receive_dio(struct in6_addr from,
                                    struct in6_addr ip6_to,
                                    const  time_t now,
                                    const u_char *dat, const int dio_len)
{
    struct nd_rpl_dio *dio = (struct nd_rpl_dio *)dat;

    if(this->packet_too_short("dio", dio_len, sizeof(*dio))) return;

    //dag_network::dump_dio(debug, dio);

    class dag_network *dn;
    if(watching) {
	dn = dag_network::find_or_make_by_dagid(dio->rpl_dagid,
						this->debug,
						watching);
    } else {
	dn = dag_network::find_by_dagid(dio->rpl_dagid);
    }

    if(dn) {
	/* and process it */
	dn->receive_dio(this, from, ip6_to, now, dio, dio_len);
    } else {
	dag_network::globalStats[PS_DIO_PACKET_IGNORED]++;
    }
}

#if 0
rpl_node *dag_network::my_dag_node(void) {
    time_t n;
    time(&n);

    if(this->node != NULL) return this->node;

    int ifindex = this->get_if_index();
    dag_network *mynet = my_dag_net();
    this->node         = mynet->find_or_make_member(if_addr);
    this->node->makevalid(if_addr, mynet, this->debug);
    this->node->set_last_seen(n);
    this->node->markself(ifindex);
    return this->node;
}
#endif

void network_interface::send_dio(dag_network *dag)
{
    unsigned char icmp_body[2048];

    debug->log("sending DIO on if: %s for prefix: %s\n",
               this->if_name, dag->prefix_name());
    memset(icmp_body, 0, sizeof(icmp_body));

    int icmp_len = dag->build_dio(icmp_body, sizeof(icmp_body),
				  dag->mPrefix);

    if(icmp_len > 0) {
	/* NULL indicates use multicast */
	this->send_raw_icmp(NULL, icmp_body, icmp_len);
    }
}

/* returns number of bytes used */
int dag_network::append_suboption(unsigned char *buff,
				  unsigned int buff_len,
				  enum RPL_SUBOPT subopt_type,
				  unsigned char *subopt_data,
				  unsigned int subopt_len)
{
    struct rpl_dio_genoption *gopt = (struct rpl_dio_genoption *)buff;
    if(buff_len < (subopt_len+2)) {
	debug->error("Failed to add option %u, length %u>avail:%u\n",
		   subopt_type, buff_len, subopt_len+2);
        return -1;
    }
    gopt->rpl_dio_type = subopt_type;
    gopt->rpl_dio_len  = subopt_len;
    memcpy(gopt->rpl_dio_data, subopt_data, subopt_len);
    return subopt_len+2;
}

int dag_network::append_suboption(unsigned char *buff,
				  unsigned int buff_len,
				  enum RPL_SUBOPT subopt_type)
{
    return append_suboption(buff, buff_len, subopt_type,
                                this->optbuff+2, this->optlen-2);
}

int dag_network::build_prefix_dioopt(ip_subnet prefix)
{
    memset(optbuff, 0, sizeof(optbuff));
    struct rpl_dio_destprefix *diodp = (struct rpl_dio_destprefix *)optbuff;

    diodp->rpl_dio_prf  = 0x00;
    diodp->rpl_dio_valid_lifetime = htonl(this->mDio_lifetime);
    diodp->rpl_dio_preferred_lifetime = htonl(this->mDio_lifetime);
    diodp->rpl_dio_prefixlen = prefix.maskbits;
    for(int i=0; i < (prefix.maskbits+7)/8; i++) {
        diodp->rpl_dio_prefix[i]=prefix.addr.u.v6.sin6_addr.s6_addr[i];
    }

    this->optlen = 30;

    return this->optlen;
}

int dag_network::build_dio(unsigned char *buff,
			   unsigned int buff_len,
			   ip_subnet prefix)
{
    struct sockaddr_in6 addr;
    struct in6_addr *dest = NULL;
    struct icmp6_hdr  *icmp6;
    struct nd_rpl_dio *dio;
    unsigned char *nextopt;
    int optlen;
    int len = 0;

    memset(buff, 0, buff_len);

    icmp6 = (struct icmp6_hdr *)buff;
    icmp6->icmp6_type = ND_RPL_MESSAGE;
    icmp6->icmp6_code = ND_RPL_DAG_IO;
    icmp6->icmp6_cksum = 0;

    dio = (struct nd_rpl_dio *)icmp6->icmp6_data8;

    dio->rpl_instanceid = mInstanceid;
    dio->rpl_version    = mVersion;
    dio->rpl_flags      = 0;
    dio->rpl_mopprf     = 0;
    if(mGrounded) {
        dio->rpl_mopprf |= ND_RPL_DIO_GROUNDED;
    }

    /* XXX need to non-storing mode is a MUST */
    dio->rpl_mopprf |= (mMode << RPL_DIO_MOP_SHIFT);

    /* XXX need to set PRF */

    dio->rpl_dtsn       = mSequence;
    dio->rpl_dagrank    = htons(mMyRank & 0xffff);
    memcpy(dio->rpl_dagid, mDagid, 16);

    nextopt = (unsigned char *)&dio[1];

    /* now announce the prefix using a destination option */
    /* this stores the option in this->optbuff */
    build_prefix_dioopt(prefix);

    int nextoptlen = 0;

    len = ((caddr_t)nextopt - (caddr_t)buff);
    nextoptlen = append_suboption(nextopt, buff_len-len, RPL_DIO_DESTPREFIX);

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
