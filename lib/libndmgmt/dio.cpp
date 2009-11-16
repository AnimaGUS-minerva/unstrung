/*
 * Copyright (C) 2009 Michael Richardson <mcr@sandelman.ca>
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
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include "oswlibs.h"
#include "rpl.h"

}

#include "iface.h"


void network_interface::receive_dio(const u_char *dio, const int dio_len)
{
        if(VERBOSE(this))
                fprintf(this->verbose_file, " processing dio(%u)\n",dio_len);
        
}

void network_interface::receive_dao(const u_char *dao, const int dao_len)
{
        if(VERBOSE(this))
                fprintf(this->verbose_file, " processing dao(%u)\n",dao_len);
        
}

void network_interface::send_dio(void)
{

}

void network_interface::send_raw_dio(unsigned char *icmp_body, unsigned int icmp_len)
{
    uint8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
    struct sockaddr_in6 addr;
    struct in6_addr *dest = NULL;
    struct in6_pktinfo *pkt_info;
    struct msghdr mhdr;
    struct cmsghdr *cmsg;
    struct iovec iov;
    char __attribute__((aligned(8))) chdr[CMSG_SPACE(sizeof(struct in6_pktinfo))];
    
    int err;
    
    setup();
    check_allrouters_membership();    

    printf("sending RA on %u\n", nd_socket);
    
    if (dest == NULL)
    {
        dest = (struct in6_addr *)all_hosts_addr;
        update_multicast_time();
    }
    
    memset((void *)&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(IPPROTO_ICMPV6);
    memcpy(&addr.sin6_addr, dest, sizeof(struct in6_addr));

    iov.iov_len  = icmp_len;
    iov.iov_base = (caddr_t) icmp_body;
    
    memset(chdr, 0, sizeof(chdr));
    cmsg = (struct cmsghdr *) chdr;
    
    cmsg->cmsg_len   = CMSG_LEN(sizeof(struct in6_pktinfo));
    cmsg->cmsg_level = IPPROTO_IPV6;
    cmsg->cmsg_type  = IPV6_PKTINFO;
    
    pkt_info = (struct in6_pktinfo *)CMSG_DATA(cmsg);
    pkt_info->ipi6_ifindex = this->get_if_index();
    memcpy(&pkt_info->ipi6_addr, &this->if_addr, sizeof(struct in6_addr));
    
#ifdef HAVE_SIN6_SCOPE_ID
    if (IN6_IS_ADDR_LINKLOCAL(&addr.sin6_addr) ||
        IN6_IS_ADDR_MC_LINKLOCAL(&addr.sin6_addr))
        addr.sin6_scope_id = iface->if_index;
#endif
    
    memset(&mhdr, 0, sizeof(mhdr));
    mhdr.msg_name = (caddr_t)&addr;
    mhdr.msg_namelen = sizeof(struct sockaddr_in6);
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = (void *) cmsg;
    mhdr.msg_controllen = sizeof(chdr);
    
    err = sendmsg(nd_socket, &mhdr, 0);
    
    if (err < 0) {
        printf("send_raw_dio/sendmsg: %s\n", strerror(errno));
    }
}

int network_interface::build_dio(unsigned char *buff,
                                 unsigned int buff_len,
                                 ip_subnet prefix)
{
    uint8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
    struct sockaddr_in6 addr;
    struct in6_addr *dest = NULL;
    struct icmp6_hdr  *icmp6;
    struct nd_rpl_dio *dio;
    int len = 0;
    
    memset(buff, 0, buff_len);
    
    icmp6 = (struct icmp6_hdr *)buff;
    icmp6->icmp6_type = ND_RPL_MESSAGE;
    icmp6->icmp6_code = ND_RPL_DAG_IO;
    icmp6->icmp6_cksum = 0;
    
    dio = (struct nd_rpl_dio *)icmp6->icmp6_data8;
    
    dio->rpl_flags = 0;
    if(this->rpl_grounded) {
        dio->rpl_flags |= ND_RPL_DIO_GROUNDED;
    }
    dio->rpl_seq        = this->rpl_sequence;
    dio->rpl_instanceid = this->rpl_instanceid;
    dio->rpl_dagrank    = this->rpl_dagrank;
    memcpy(dio->rpl_dagid, this->rpl_dagid, 16);
    
    len = ((caddr_t)&dio[1] - (caddr_t)buff);
    
    return len;
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
