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

static void format_dagid(char *dagidstr, u_int8_t rpl_dagid[DAGID_LEN])
{
    char *d = dagidstr;
    bool lastwasnull=false;
    int  i;

    /* should attempt to format as IPv6 address too */
    for(i=0;i<16;i++) {
        if(isprint(rpl_dagid[i])) {
            *d++ = rpl_dagid[i];
            lastwasnull=false;
        } else {
            if(rpl_dagid[i] != '\0' || !lastwasnull) {
                int cnt=snprintf(d, 6,"0x%02x ", rpl_dagid[i]);
                d += strlen(d);
            }
            lastwasnull = (rpl_dagid[i] == '\0');
        }
    }
    *d++ = '\0';
}

void network_interface::receive_dio(struct in6_addr from,
                                    const  time_t now,
                                    const u_char *dat, const int dio_len)
{
    if(debug->verbose_test()) {
        debug->verbose(" processing dio(%u)\n",dio_len);
    }

    struct nd_rpl_dio *dio = (struct nd_rpl_dio *)dat;

    if(this->packet_too_short("dio", dio_len, sizeof(*dio))) return;

    char dagid[16*6];
    format_dagid(dagid, dio->rpl_dagid);
    debug->info(" [seq:%u,instance:%u,rank:%u,version:%u,%s%s,dagid:%s]\n",
                dio->rpl_dtsn, dio->rpl_instanceid, ntohs(dio->rpl_dagrank),
                dio->rpl_version,
                (RPL_DIO_GROUNDED(dio->rpl_mopprf) ? "grounded," : ""),
                dag_network::mop_decode(dag_network::mop_extract(dio)),
                dagid);
    
    /* find the relevant DAG */
    class dag_network *dn = dag_network::find_or_make_by_dagid(dio->rpl_dagid,
                                                               this->debug);

    /* and process it */
    dn->receive_dio(this, from, now, dio, dio_len);
}

rpl_node *network_interface::my_dag_node(void) {
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

dag_network *network_interface::my_dag_net(void) {
    if(this->dagnet != NULL) return this->dagnet;

    this->dagnet = dag_network::find_or_make_by_dagid(rpl_dagid,debug);
    return this->dagnet;
}

void network_interface::set_rpl_prefix(const ip_subnet prefix) {
    rpl_prefix = prefix;
    subnettot(&prefix, 0, rpl_prefix_str, sizeof(rpl_prefix_str));

    rpl_node    *myself= my_dag_node();
    dag_network *mynet = my_dag_net();
    if(mynet != NULL && myself != NULL) {
        mynet->addprefix(*myself, this, prefix);
    }
}

void network_interface::set_rpl_interval(const int msec) 
{
    rpl_interval_msec = msec;
    rpl_event *re  = new rpl_event(0, msec, rpl_event::rpl_send_dio,
                                   if_name, this->debug);
    re->event_type = rpl_event::rpl_send_dio;

    re->interface = this;
    re->requeue();
    //this->dio_event = re;        // needs to be smart-pointer
}

void network_interface::send_dio(void)
{
    unsigned char icmp_body[2048];

    debug->log("sending DIO on if: %s for prefix: %s\n",
               this->if_name, this->rpl_prefix_str);
    memset(icmp_body, 0, sizeof(icmp_body));
    
    unsigned int icmp_len = build_dio(icmp_body, sizeof(icmp_body),
                                      rpl_prefix);

    send_raw_icmp(icmp_body, icmp_len);
}


void network_interface::send_raw_icmp(unsigned char *icmp_body, unsigned int icmp_len)
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
    
    if(setup() == false) {
        fprintf(this->verbose_file, "failed to setup socket!");
        return;
    }
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
        addr.sin6_scope_id = iface->get_if_index();
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
        char sbuf[INET6_ADDRSTRLEN], dbuf[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, &pkt_info->ipi6_addr, sbuf, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &addr.sin6_addr, dbuf, INET6_ADDRSTRLEN);
        
        printf("send_raw_dio/sendmsg[%s->%s] (on if: %d): %s\n",
               sbuf, dbuf,
               pkt_info->ipi6_ifindex, strerror(errno));
    }
}

/* returns number of bytes used */
int network_interface::append_suboption(unsigned char *buff,
                                            unsigned int buff_len,
                                            enum RPL_SUBOPT subopt_type,
                                            unsigned char *subopt_data,
                                            unsigned int subopt_len)
{
    struct rpl_dio_genoption *gopt = (struct rpl_dio_genoption *)buff;
    if(buff_len < (subopt_len+2)) {
        fprintf(this->verbose_file, "Failed to add option %u, length %u>avail:%u\n",
                subopt_type, buff_len, subopt_len+2);
        return -1;
    }
    gopt->rpl_dio_type = subopt_type;
    gopt->rpl_dio_len  = subopt_len;
    memcpy(gopt->rpl_dio_data, subopt_data, subopt_len);
    return subopt_len+2;
}

int network_interface::append_suboption(unsigned char *buff,
                                            unsigned int buff_len,
                                            enum RPL_SUBOPT subopt_type)
{
    return append_suboption(buff, buff_len, subopt_type,
                                this->optbuff+2, this->optlen-2);
}

int network_interface::build_prefix_dioopt(ip_subnet prefix)
{
    memset(optbuff, 0, sizeof(optbuff));
    struct rpl_dio_destprefix *diodp = (struct rpl_dio_destprefix *)optbuff;

    diodp->rpl_dio_prf  = 0x00;
    diodp->rpl_dio_prefixlifetime = htonl(this->rpl_dio_lifetime);
    diodp->rpl_dio_prefixlen = prefix.maskbits;
    for(int i=0; i < (prefix.maskbits+7)/8; i++) {
        diodp->rpl_dio_prefix[i]=prefix.addr.u.v6.sin6_addr.s6_addr[i];
    }

    this->optlen = ((prefix.maskbits+7)/8 + 1 + 4 + 4);

    return this->optlen;
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
    unsigned char *nextopt;
    int optlen;
    int len = 0;
    
    memset(buff, 0, buff_len);
    
    icmp6 = (struct icmp6_hdr *)buff;
    icmp6->icmp6_type = ND_RPL_MESSAGE;
    icmp6->icmp6_code = ND_RPL_DAG_IO;
    icmp6->icmp6_cksum = 0;
    
    dio = (struct nd_rpl_dio *)icmp6->icmp6_data8;
    
    dio->rpl_instanceid = this->rpl_instanceid;
    dio->rpl_version    = this->rpl_version;
    dio->rpl_flags = 0;
    dio->rpl_mopprf     = 0;
    if(this->rpl_grounded) {
        dio->rpl_mopprf |= ND_RPL_DIO_GROUNDED;
    }
    
    /* XXX need to non-storing mode is a MUST */
    dio->rpl_mopprf |= (rpl_mode << RPL_DIO_MOP_SHIFT);

    /* XXX need to set PRF */

    dio->rpl_dtsn       = this->rpl_sequence;
    dio->rpl_dagrank    = htons(this->rpl_dagrank);
    memcpy(dio->rpl_dagid, this->rpl_dagid, 16);

    nextopt = (unsigned char *)&dio[1];
    
    /* now announce the prefix using a destination option */
    /* this stores the option in this->optbuff */
    build_prefix_dioopt(prefix);

    int nextoptlen = 0;

    len = ((caddr_t)nextopt - (caddr_t)buff);
    nextoptlen += append_suboption(nextopt, buff_len-len, RPL_DIO_DESTPREFIX);

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
 * here are routines to crack open DIO messages
 */
rpl_dio::rpl_dio(rpl_node &peer,
                 const struct nd_rpl_dio *dio, int dio_len) :
    mPeer(peer)
{
    reset_options();

    if(dio_len > sizeof(mBytes)) dio_len = sizeof(mBytes);
    memcpy(mBytes, dio, dio_len);
    mLen = dio_len;
}

void rpl_dio::reset_options(void)
{
    /* clear options */
    for(int i=0; i<256; i++) mOptions[i]=0;
}

/*
 * This routine looks for another option of a particular type.
 * If mOptions[optnum] is 0, then we start from the beginning.
 * If mOptions[optnum] is -1, then we found everything.
 * otherwise, we start scanning at that offset.
 *
 * If we find a different option, and mOptions[subopt] is 0, then
 * we update it to that location.
 */

struct nd_rpl_genoption *rpl_dio::search_subopt(enum RPL_SUBOPT optnum,
                                                int *p_opt_len)
{
    u_int8_t*opt_bytes = (mBytes+sizeof(nd_rpl_dio));
    int opt_left_len = mLen-sizeof(struct nd_rpl_dio);
    int offset = 0;

    if(p_opt_len) *p_opt_len = 0;

    /* if we have already scanned to end of options... */
    if(mOptions[optnum] == -1) return NULL;
    
    /* else initialize to mOptions[optnum] */
    offset       += mOptions[optnum];
    opt_bytes    += offset;
    opt_left_len -= mOptions[optnum];

    while(opt_left_len > sizeof(struct rpl_dio_genoption)) {
        int skip_len = 0;

        /* make sure we do not do unaligned accesses */
        struct rpl_dio_genoption opt;
        memcpy((void*)&opt, (void*)opt_bytes,sizeof(struct rpl_dio_genoption));

        if(opt.rpl_dio_type == RPL_OPT_PAD0) {
            skip_len = 1;
        } else {
            skip_len = opt.rpl_dio_len + 2;
            if(opt.rpl_dio_len == 0) break;
        }

        /* if mOptions[dio_type] is 0, then advance it */
        if(mOptions[opt.rpl_dio_type]==0) {
            mOptions[opt.rpl_dio_type]=offset;
        }

        /* see if we found what we were looking for... ? */
        if(opt.rpl_dio_type == optnum) {
            mOptions[opt.rpl_dio_type]=offset+skip_len;
            if(p_opt_len) *p_opt_len = skip_len;
            return (struct nd_rpl_genoption *)opt_bytes;
        }

        opt_left_len -= skip_len;
        opt_bytes    += skip_len;
        offset       += skip_len;
    }
    return NULL;
}

struct rpl_dio_destprefix *rpl_dio::destprefix(void)
{
    int optlen = 0;
    struct rpl_dio_destprefix *dp = (struct rpl_dio_destprefix *)search_subopt(RPL_DIO_DESTPREFIX, &optlen);

    if(dp==NULL) return NULL;

    int prefixbytes = ((dp->rpl_dio_prefixlen+7) / 8)-1;
    if(prefixbytes > (optlen - sizeof(struct rpl_dio_destprefix))) {
        //(*mStats)[PS_SUBOPTION_UNDERRUN]++;
        return NULL;
    }
    
    return dp;
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
