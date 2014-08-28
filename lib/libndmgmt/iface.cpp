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

/*
 * this file is just for setup and maintenance of interface,
 * it does not really do any heavy lifting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include <time.h>
#include "rpl.h"
#include "hexdump.c"

#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL   0x0020U
#endif
}

#include "iface.h"
#include "dag.h"

bool                  network_interface::signal_usr2;
bool                  network_interface::faked_time;
bool                  network_interface::terminating_soon = false;
struct timeval        network_interface::fake_time;
class rpl_event_queue network_interface::things_to_do;

class network_interface *loopback_interface = NULL;

const uint8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,0x01};
const uint8_t all_rpl_addr[]   = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,0x1a};


network_interface::network_interface()
{
    nd_socket = -1;
    alive = false;
    debug = NULL;
    this->set_if_name("<unset>");
}

network_interface::network_interface(const char *if_name, rpl_debug *deb)
{
    alive = false;
    nd_socket = -1;
    set_debug(deb);

    this->set_if_name(if_name);
    debug = NULL;
}

void network_interface::set_if_name(const char *if_name)
{
    this->if_name[0]=0;
    strncat(this->if_name, if_name, sizeof(this->if_name));
}

void network_interface::generate_linkaddr(void)
{
    memset(ipv6_link_addr.s6_addr, 0, 16);
    ipv6_link_addr.s6_addr[0]=0xfe;
    ipv6_link_addr.s6_addr[1]=0x80;
    memcpy(ipv6_link_addr.s6_addr+8, eui64, 8);
}

void network_interface::generate_eui64(void)
{
    if(eui64[0]!=0) return;
    eui64set=true;

    /*
     * eui64 is upper 3 bytes of eui48, with bit 0x02 set to indicate
     * that it is a "globabally" generated eui64
     *
     * then 0xfffe as bytes 4/5, and then lower 3 bytes of eui48
     *
     * maybe NETLINK should provide us with the EUI64?
     */
    eui64[0]=eui48[0] | 0x02;
    eui64[1]=eui48[1];
    eui64[2]=eui48[2];
    eui64[3]=0xff;
    eui64[4]=0xfe;
    eui64[5]=eui48[3];
    eui64[6]=eui48[4];
    eui64[7]=eui48[5];

    generate_linkaddr();
}

char *network_interface::eui64_str(char *str, int strlen)
{
    snprintf(str, strlen, "%02x%02x:%02x%02x:%02x%02x:%02x%02x",
             eui64[0], eui64[1], eui64[2], eui64[3],
             eui64[4], eui64[5], eui64[6], eui64[7]);
    return str;
}

char *network_interface::eui48_str(char *str, int strlen)
{
    snprintf(str, strlen, "%02x:%02x:%02x:%02x:%02x:%02x",
             eui48[0], eui48[1], eui48[2], eui48[3],
             eui48[4], eui48[5]);
    return str;
}


bool network_interface::setup()
{
    if(alive && nd_socket != -1) return true;

    generate_eui64();

    debug->verbose("Starting setup for %s\n", this->if_name);

    nd_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    struct icmp6_filter filter;
    int err, val;

    if (nd_socket < 0)
    {
        debug->error("can't create socket(AF_INET6): %s", strerror(errno));
	return false;
    }

    val = 1;
    err = setsockopt(nd_socket, IPPROTO_IPV6, IPV6_RECVPKTINFO, &val, sizeof(val));
    if (err < 0)
    {
        debug->error("setsockopt(IPV6_RECVPKTINFO): %s", strerror(errno));
        return false;
    }

    val = 2;
    err = setsockopt(nd_socket, IPPROTO_RAW, IPV6_CHECKSUM, &val, sizeof(val));
    if (err < 0)
    {
        debug->error("setsockopt(IPV6_CHECKSUM): %s", strerror(errno));
        return false;
    }

    val = 255;
    err = setsockopt(nd_socket, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &val, sizeof(val));
    if (err < 0)
    {
        debug->error("setsockopt(IPV6_UNICAST_HOPS): %s", strerror(errno));
        return false;
    }

    val = 255;
    err = setsockopt(nd_socket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &val, sizeof(val));
    if (err < 0)
    {
        debug->error("setsockopt(IPV6_MULTICAST_HOPS): %s", strerror(errno));
        return false;
    }

#ifdef IPV6_RECVHOPLIMIT
    val = 1;
    err = setsockopt(nd_socket, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &val, sizeof(val));
    if (err < 0)
    {
        debug->error("setsockopt(IPV6_RECVHOPLIMIT): %s", strerror(errno));
        return false;
    }
#endif

#if 1
    /*
     * setup ICMP filter
     */

    ICMP6_FILTER_SETBLOCKALL(&filter);
    ICMP6_FILTER_SETPASS(ND_RPL_MESSAGE,    &filter);
    ICMP6_FILTER_SETPASS(ND_ROUTER_SOLICIT, &filter);
    ICMP6_FILTER_SETPASS(ND_ROUTER_ADVERT,  &filter);

    err = setsockopt(nd_socket, IPPROTO_ICMPV6, ICMP6_FILTER, &filter,
                     sizeof(filter));
    if (err < 0)
    {
        debug->error("setsockopt(ICMPV6_FILTER): %s", strerror(errno));
        return false;
    }
#endif

    //setup_allrouters_membership();
    setup_allrpl_membership();
    struct ipv6_mreq mreq;

    /* make sure that there is an rpl_node entry for ourselves */
    /* XXX rpl_node should include all this nodes' addresses, on
     *     all of this nodes' interface
     */

    alive = true;
    add_to_list();

    return true;
}

void network_interface::setup_allrouters_membership(void)
{
	struct ipv6_mreq mreq;

	memset(&mreq, 0, sizeof(mreq));
	mreq.ipv6mr_interface = this->get_if_index();

	/* ipv6-allrouters: ff02::2 */
        memcpy(&mreq.ipv6mr_multiaddr.s6_addr[0], all_hosts_addr, sizeof(mreq.ipv6mr_multiaddr));

        if (setsockopt(nd_socket, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        {
            /* linux-2.6.12-bk4 returns error with HUP signal but keep listening */
            if (errno != EADDRINUSE)
            {
                debug->error("can not join ipv6-allrouters on %s: %s\n",
                             this->if_name, strerror(errno));
                alive = false;
                return;
            }
        }

	return;
}

void network_interface::setup_allrpl_membership(void)
{
	struct ipv6_mreq mreq;

	memset(&mreq, 0, sizeof(mreq));
	mreq.ipv6mr_interface = this->get_if_index();

	/* all-rpl-nodes: ff02::1a */
        memcpy(&mreq.ipv6mr_multiaddr.s6_addr[0], all_rpl_addr, sizeof(mreq.ipv6mr_multiaddr));

        if (setsockopt(nd_socket, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        {
            /* linux-2.6.12-bk4 returns error with HUP signal but keep listening */
            if (errno != EADDRINUSE)
            {
                debug->warn("can not join ipv6-all-rpl-nodes on %s: %s\n",
                            this->if_name, strerror(errno));
                alive = false;
                return;
            }
        }

	return;
}

void network_interface::check_allrouters_membership(void)
{
	#define ALL_ROUTERS_MCAST "ff020000000000000000000000000002"
	#define ALL_RPL_MCAST     "ff02000000000000000000000000001a"

	FILE *fp;
	unsigned int if_idx, allrouters_ok=0, allrpl_ok=0;
	char addr[32+1];
	int ret=0;
        int lineno=0;

	if ((fp = fopen(PATH_PROC_NET_IGMP6, "r")) == NULL)
	{
		printf("can't open %s: %s", PATH_PROC_NET_IGMP6,
			strerror(errno));
		return;
	}

	while ( (ret=fscanf(fp, "%u %*s %32[0-9A-Fa-f] %*x %*x %*x\n", &if_idx, addr)) != EOF) {
            lineno++;
#if 0
            printf("processing line %u with ret=%u\n",
                   lineno, ret);
#endif
            if (ret == 2) {
                if (this->if_index == if_idx) {
                    if (strncmp(addr, ALL_ROUTERS_MCAST, sizeof(addr)) == 0)
                        allrouters_ok = 1;
                    if (strncmp(addr, ALL_RPL_MCAST, sizeof(addr)) == 0)
                        allrpl_ok = 1;
                }
            }
	}

	fclose(fp);

	if (!allrouters_ok) {
            printf("resetting ipv6-allrouters membership on %s\n", this->if_name);
            setup_allrouters_membership();
	}

	if (!allrpl_ok) {
            printf("resetting ipv6-rpl membership on %s\n", this->if_name);
            setup_allrpl_membership();
	}

	return;
}

class dag_network *network_interface::find_or_make_dag_by_dagid(const char *name)
{
        return dag_network::find_or_make_by_string(name, debug, false);
}


/* XXX replace with STL list */
class network_interface *network_interface::all_if = NULL;
void network_interface::add_to_list(void)
{
    if(on_list) return;

    this->next = network_interface::all_if;
    network_interface::all_if = this;

    on_list = true;
}

network_interface *network_interface::find_by_ifindex(int index)
{
    network_interface *ni = network_interface::all_if;
    while(ni!=NULL && ni->if_index!=index) ni=ni->next;
    return ni;
}

network_interface *network_interface::find_by_name(const char *name)
{
    network_interface *ni = network_interface::all_if;
    while(ni!=NULL && strcasecmp(name, ni->if_name)!=0) ni=ni->next;
    return ni;
}

int network_interface::foreach_if(int (*func)(network_interface *, void *arg),
                                   void *arg)
{
    int ret=-1;
    network_interface *ni = network_interface::all_if;

    while(ni!=NULL) {
        network_interface *next = ni->next;
        ret = (*func)(ni, arg);

        if(ret==0) return ret;
        ni=next;
    }
    return ret;
}

static int remove_mark(network_interface *ni, void *arg)
{
    ni->mark = false;
    return 1;
}

void network_interface::remove_marks(void)
{
    foreach_if(remove_mark, NULL);
}




/* this searches through /proc to get right ifindex */
/* probably do not need this anymore, now that we use netlink */
int network_interface::get_if_index(void)
{
    if(this->if_index > 0) {
        return this->if_index;
    }

    /*
     * this function extracts the link local address and interface index
     * from PATH_PROC_NET_IF_INET6.
     */
    FILE *fp;
    char str_addr[40];
    unsigned int plen, scope, dad_status, if_idx;
    char devname[IFNAMSIZ];

    if ((fp = fopen(PATH_PROC_NET_IF_INET6, "r")) == NULL)
    {
        debug->error("can't open %s: %s", PATH_PROC_NET_IF_INET6,
                strerror(errno));
        return -1;
    }

    while (fscanf(fp, "%32s %x %02x %02x %02x %15s\n",
                  str_addr, &if_idx, &plen, &scope, &dad_status,
                  devname) != EOF)
    {
        if (scope == IPV6_ADDR_LINKLOCAL &&
            strcmp(devname, this->if_name) == 0)
        {
            struct in6_addr addr;
            unsigned int ap;
            int i;

            for (i=0; i<16; i++)
            {
                sscanf(str_addr + i * 2, "%02x", &ap);
                addr.s6_addr[i] = (unsigned char)ap;
            }
            memcpy(&this->if_addr, &addr, sizeof(this->if_addr));

            this->if_index = if_idx;
            fclose(fp);
            return this->if_index;
        }
    }

    debug->error("no linklocal address configured for %s",
            this->if_name);
    fclose(fp);
    return -1;
}

void network_interface::receive_packet(struct in6_addr ip6_src,
				       struct in6_addr ip6_dst,
                                       const  time_t   now,
				       const u_char *bytes, const int len)
{
    const struct nd_opt_hdr *nd_options = NULL;
    char src_addrbuf[INET6_ADDRSTRLEN];
    char dst_addrbuf[INET6_ADDRSTRLEN];
    logged = false;

    /* should collect this all into a "class packet", or "class transaction" */
    if(this->debug->verbose_test()) {
	inet_ntop(AF_INET6, &ip6_src, src_addrbuf, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &ip6_dst, dst_addrbuf, INET6_ADDRSTRLEN);
    }

    struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)bytes;

    /* mark end of data received */
    const u_char *bytes_end = bytes + len;

    /* XXX should maybe check the checksum? */

    if(debug->flag_set(RPL_DEBUG_NETINPUT)) {
        this->log_received_packet(ip6_src, ip6_dst);
    }

    switch(icmp6->icmp6_type) {
    case ND_RPL_MESSAGE:
        switch(icmp6->icmp6_code) {
        case ND_RPL_DAG_IO:
            this->receive_dio(ip6_src, ip6_dst, now,
                              icmp6->icmp6_data8, bytes_end - icmp6->icmp6_data8);
            break;

        case ND_RPL_DAO:
            this->receive_dao(ip6_src, ip6_dst, now,
                              icmp6->icmp6_data8, bytes_end - icmp6->icmp6_data8);
            break;

        case ND_RPL_DAO_ACK:
            this->receive_daoack(ip6_src, ip6_dst, now,
                                 icmp6->icmp6_data8, bytes_end - icmp6->icmp6_data8);
            break;

        default:
            this->log_received_packet(ip6_src, ip6_dst);
            debug->warn("Got unknown RPL code: %u\n", icmp6->icmp6_code);
            break;
        }
        return;

    case ND_ROUTER_SOLICIT:
    {
	struct nd_router_solicit *nrs = (struct nd_router_solicit *)bytes;
	nd_options = (const struct nd_opt_hdr *)&nrs[1];
        this->log_received_packet(ip6_src, ip6_dst);
	debug->warn("Got router solicitation from %s with %u bytes of options\n",
		    src_addrbuf,
		    bytes_end - (u_char *)nd_options);
    }
    break;

    case ND_ROUTER_ADVERT:
    {
	struct nd_router_advert *nra = (struct nd_router_advert *)bytes;
	nd_options = (const struct nd_opt_hdr *)&nra[1];
	if(debug->verbose_test()) {
            this->log_received_packet(ip6_src, ip6_dst);
	    debug->verbose("Got router advertisement from %s \n"
                           "\t(hoplimit: %u, flags=%s%s%s, lifetime=%ums, reachable=%u, retransmit=%u)\n"
                           "\twith %u bytes of options\n",
		    src_addrbuf,
		    nra->nd_ra_curhoplimit,
		    nra->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED ? "managed " : "",
		    nra->nd_ra_flags_reserved & ND_RA_FLAG_OTHER ? "other " : "",
		    nra->nd_ra_flags_reserved & ND_RA_FLAG_HOME_AGENT ? "home-agent " : "",
		    ntohs(nra->nd_ra_router_lifetime),
		    ntohl(nra->nd_ra_reachable),
		    ntohl(nra->nd_ra_retransmit),
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;

    case ND_NEIGHBOR_SOLICIT:
    {
	struct nd_neighbor_solicit *nns = (struct nd_neighbor_solicit *)bytes;
	nd_options = (const struct nd_opt_hdr *)&nns[1];
	if(debug->verbose_test()) {
            this->log_received_packet(ip6_src, ip6_dst);

	    char target_addrbuf[INET6_ADDRSTRLEN];
	    inet_ntop(AF_INET6, &nns->nd_ns_target, target_addrbuf, INET6_ADDRSTRLEN);
	    debug->verbose("Got neighbour solicitation from %s, looking for %s, has %u bytes of options\n",
		    src_addrbuf,
		    target_addrbuf,
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;

    case ND_NEIGHBOR_ADVERT:
    {
	struct nd_neighbor_advert *nna = (struct nd_neighbor_advert *)bytes;
	nd_options = (const struct nd_opt_hdr *)&nna[1];
	if(debug->verbose_test()) {
            this->log_received_packet(ip6_src, ip6_dst);

	    char target_addrbuf[INET6_ADDRSTRLEN];
	    inet_ntop(AF_INET6, &nna->nd_na_target, target_addrbuf, INET6_ADDRSTRLEN);
            debug->verbose("Got neighbor advertisement from %s (flags=%s%s%s), advertising: %s, has %u bytes of options\n",
		    src_addrbuf,
		    nna->nd_na_flags_reserved & ND_NA_FLAG_ROUTER ? "router " : "",
		    nna->nd_na_flags_reserved & ND_NA_FLAG_SOLICITED ? "solicited " : "",
		    nna->nd_na_flags_reserved & ND_NA_FLAG_OVERRIDE  ? "override " : "",

		    target_addrbuf,
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;
    default:
        this->log_received_packet(ip6_src, ip6_dst); logged=true;
        return;
    }

#if 1
    /* now decode the option packets */
    while(nd_options != NULL && (const u_char *)nd_options < bytes_end && nd_options->nd_opt_type!=0) {
	/*
	 * option lengths are in units of 64-bits, the minimum size for
	 * an IPv6 extension
	 */
	unsigned int optlen = nd_options->nd_opt_len << 3;
	if(this->packet_too_short("option",
				  optlen,
				  sizeof(struct nd_opt_hdr))) return;

	if(this->packet_too_short("option",
				  (bytes_end - bytes),
				  optlen)) return;

	if(debug->verbose_test()) {
            debug->verbose_more("    option %u[%u]",
                                nd_options->nd_opt_type, optlen);
	}
	switch(nd_options->nd_opt_type) {
	case ND_OPT_SOURCE_LINKADDR:
            debug->verbose_more(" source-linkaddr \n");
	    break;

	case ND_OPT_TARGET_LINKADDR:
	    debug->verbose_more(" target-linkaddr \n");
	    break;

	case ND_OPT_PREFIX_INFORMATION:
	{
	    struct nd_opt_prefix_info *nopi = (struct nd_opt_prefix_info *)nd_options;
	    if(this->packet_too_short("option",
				      optlen,
				      sizeof(struct nd_opt_prefix_info))) return;

	    if(debug->verbose_test()) {
		char prefix_addrbuf[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &nopi->nd_opt_pi_prefix, prefix_addrbuf, INET6_ADDRSTRLEN);
                debug->verbose_more(" prefix %s/%u (valid=%us, preferred=%us) flags=%s%s%s\n",
			prefix_addrbuf, nopi->nd_opt_pi_prefix_len,
			nopi->nd_opt_pi_valid_time,
			nopi->nd_opt_pi_preferred_time,
			nopi->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_ONLINK ? "onlink " : "",
			nopi->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_AUTO   ? "auto " : "",
			nopi->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_RADDR  ? "raddr " : "");
	    }
	}
	break;

	case ND_OPT_REDIRECTED_HEADER:
	    debug->verbose_more(" redirected \n");
	    break;

	case ND_OPT_MTU:
            debug->verbose_more(" mtu \n");
	    break;

	case ND_OPT_RTR_ADV_INTERVAL:
	    debug->verbose_more(" rtr-interval \n");
	    break;

	case ND_OPT_HOME_AGENT_INFO:
	    debug->verbose_more(" home-agent-info \n");
	    break;

	default:
	    /* nothing */
	    break;
	}

	const u_char *nd_bytes = (u_char *)nd_options;
	nd_bytes += optlen;
	nd_options = (const struct nd_opt_hdr *)nd_bytes;
    }
    /* Dead code? */
#endif
    /* terminate the debug line */
    debug->verbose("");
}

int network_interface::packet_too_short(const char *thing,
				       const int avail_len,
				       const int needed_len)
{
    if(avail_len < needed_len) {
        debug->warn("%s has invalid len: %u(<%u)\n",
		thing, avail_len, needed_len);

	this->error_cnt++;
	/* discard packet */
	return 1;
    }
    return 0;
}

void network_interface::receive(const time_t now)
{
    unsigned char b[2048];
    struct sockaddr_in6 src_sock;
    struct in6_addr src, dst;
    int len, n;
    int recv_ifindex;
    struct msghdr mhdr;
    struct iovec iov;
    struct in6_pktinfo *pkt_info;

    if( ! control_msg_hdr )
    {
        control_msg_hdrlen = CMSG_SPACE(sizeof(struct in6_pktinfo)) +
            CMSG_SPACE(sizeof(int));
        if ((control_msg_hdr = (unsigned char *)malloc(control_msg_hdrlen)) == NULL) {
            debug->error("recv_rs_ra: malloc: %s", strerror(errno));
            return;
        }
    }

    iov.iov_len = sizeof(b);
    iov.iov_base = (caddr_t) b;

    memset(&mhdr, 0, sizeof(mhdr));
    mhdr.msg_name = (caddr_t)&src_sock;
    mhdr.msg_namelen = sizeof(src_sock);
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = (void *)control_msg_hdr;
    mhdr.msg_controllen = control_msg_hdrlen;

    if((len = recvmsg(this->nd_socket, &mhdr, 0)) >= 0) {
        struct cmsghdr *cmsg;

        memset(&dst, 0, sizeof(dst));
        hoplimit = -1;

        for (cmsg = CMSG_FIRSTHDR(&mhdr); cmsg != NULL; cmsg = CMSG_NXTHDR(&mhdr, cmsg))
	{
            if (cmsg->cmsg_level != IPPROTO_IPV6)
          	continue;

            switch(cmsg->cmsg_type)
            {
#ifdef IPV6_HOPLIMIT
            case IPV6_HOPLIMIT:
                if ((cmsg->cmsg_len == CMSG_LEN(sizeof(int))) &&
                    (*(int *)CMSG_DATA(cmsg) >= 0) &&
                    (*(int *)CMSG_DATA(cmsg) < 256))
                {
                    hoplimit = *(int *)CMSG_DATA(cmsg);
                }
                else
                {
                    debug->warn("received a bogus IPV6_HOPLIMIT from the kernel! len=%d, data=%d",
                               cmsg->cmsg_len, *(int *)CMSG_DATA(cmsg));
                    return;
                }
                break;
#endif /* IPV6_HOPLIMIT */

            case IPV6_PKTINFO:
                if ((cmsg->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo))) &&
                    ((struct in6_pktinfo *)CMSG_DATA(cmsg))->ipi6_ifindex)
                {
                    pkt_info = (struct in6_pktinfo *)CMSG_DATA(cmsg);
                }
                else
                {
                    debug->warn("received a bogus IPV6_PKTINFO from the kernel! len=%d, index=%d",
                               cmsg->cmsg_len, ((struct in6_pktinfo *)CMSG_DATA(cmsg))->ipi6_ifindex);
                    return;
                }
                break;
            }
	}

        recv_ifindex = if_index;    /* default value */
        if(pkt_info) {
            dst     = pkt_info->ipi6_addr;
            recv_ifindex = pkt_info->ipi6_ifindex;
        }
        src = src_sock.sin6_addr;

        /*
         * multicast sockets will receive one per interface; the ifindex
         * is the accurate view of where it was received, so we have to
         * switch interfaces if this->ifindex !+ ifindex
         */
        network_interface *iface = this;
        if(recv_ifindex != if_index) {
            iface = find_by_ifindex(recv_ifindex);
        }

        iface->receive_packet(src, dst, now, b, len);
    }

}

void network_interface::log_received_packet(struct in6_addr src,
                                            struct in6_addr dst)
{
    if(!logged) {
        char src_addrbuf[INET6_ADDRSTRLEN];
        char dst_addrbuf[INET6_ADDRSTRLEN];

        inet_ntop(AF_INET6, &src, src_addrbuf, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, &dst, dst_addrbuf, INET6_ADDRSTRLEN);

        debug->verbose(" %s: received packet from %s -> %s[%u] hoplimit=%d\n",
                       if_name,
                       src_addrbuf, dst_addrbuf,
                       if_index, hoplimit);
        logged = true;
    }
}


void network_interface::send_raw_icmp(struct in6_addr *dest,
                                      const unsigned char *icmp_body,
                                      const unsigned int icmp_len)
{
    uint8_t all_rpl_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,0x1a};
    struct sockaddr_in6 addr;
    struct in6_pktinfo *pkt_info;
    struct msghdr mhdr;
    struct cmsghdr *cmsg;
    struct iovec iov;
    char __attribute__((aligned(8))) chdr[CMSG_SPACE(sizeof(struct in6_pktinfo))];

    int err;

#if 1
    if(setup() == false) {
        debug->error("failed to setup socket!");
        return;
    }
    check_allrouters_membership();
#endif

    if (dest == NULL)
    {
        dest = (struct in6_addr *)all_rpl_addr;  /* fixed to official ff02::1a */
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

    err = sendmsg(this->nd_socket, &mhdr, 0);

    if (err < 0) {
        char sbuf[INET6_ADDRSTRLEN], dbuf[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, &pkt_info->ipi6_addr, sbuf, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &addr.sin6_addr, dbuf, INET6_ADDRSTRLEN);

        debug->info("send_raw_dio/sendmsg[%s->%s] (on if: %d): %s\n",
                    sbuf, dbuf,
                    pkt_info->ipi6_ifindex, strerror(errno));
    }
}

int network_interface::if_count(void)
{
    class network_interface *iface = network_interface::all_if;
    int count = 0;

    while(iface != NULL) {
        count++;
        iface = iface->next;
    }
    return count;
}

/* this runs the next event, even if it is not time yet */
void network_interface::terminating(void) {
    terminating_soon = true;
}

/* this runs the next event, even if it is not time yet */
bool network_interface::force_next_event(void) {
    rpl_event *re = things_to_do.next_event();

    struct timeval now;
    gettimeofday(&now, NULL);

    if(re) {
	if(re->doit() && !terminating_soon) {
            re->requeue(now);
	} else {
	    delete re;
	}
        return true;
    } else {
        return false;
    }
}

/* empty all events */
void network_interface::clear_events(void) {
    rpl_event *re;
    while((re = things_to_do.next_event()) != NULL) {
	delete re;
    }
}

void network_interface::catch_signal_usr2(int signum,
						 siginfo_t *si,
						 void *ucontext)
{
    /* should be atomic */
    network_interface::signal_usr2 = true;
}



void network_interface::main_loop(FILE *verbose, rpl_debug *debug)
{
    bool done = false;

    struct sigaction usr2;
    usr2.sa_sigaction = catch_signal_usr2;
    usr2.sa_flags = SA_SIGINFO|SA_RESTART;

    if(sigaction(SIGUSR2, &usr2, NULL) != 0) {
	perror("sigaction USR2");
    }

    while(!done) {
        struct pollfd            poll_if[network_interface::if_count()];
        class network_interface* all_if[network_interface::if_count()];
        int pollnum=0;
        int timeout = 60*1000;   /* 60 seconds is maximum */

        struct timeval now;
        gettimeofday(&now, NULL);

	debug->verbose2("checking things to do list, has %d items\n",
			things_to_do.size());

        rpl_event *re = things_to_do.peek_event();
        while((re = things_to_do.peek_event()) != NULL) {
            if(re->passed(now)) {
		things_to_do.eat_event();
                if(re->doit()) {
                    re->requeue(now);
                } else {
                    delete re;
                }
            } else {
                // since things are sorted, when we find something which
                // has not yet passed, then it must be in the future.
                int newtimeout = re->miliseconds_util(now);
                if(newtimeout < 0) {
		    debug->warn("negative timeout %d\n", newtimeout);
                } else if(newtimeout < timeout) timeout = newtimeout;
                break;
            }
        }

        /*
         * do not really need to build this every time, but, for
         * now, this is fine.
         */
        class network_interface *iface = network_interface::all_if;
        while(iface != NULL) {
            debug->verbose2("iface %s has socketfd: %d\n",
			   iface->if_name, iface->nd_socket);
            if(iface->nd_socket != -1) {
                poll_if[pollnum].fd = iface->nd_socket;
                poll_if[pollnum].events = POLLIN;
                poll_if[pollnum].revents= 0;
                all_if[pollnum] = iface;

                pollnum++;
            }
            iface = iface->next;
        }

        /* now poll for input */
        debug->verbose2("sleeping with %d file descriptors, for %d ms\n",
			pollnum, timeout);
#if 0
	if(debug->flag) {
	    things_to_do.printevents(debug->file, "loop");
	}
#endif

        int n = poll(poll_if, pollnum, timeout);

        if(n == 0) {
            /* there was a timeout */
        } else if(n > 0) {
            /* there is data ready */
            for(int i=0; i < pollnum && n > 0; i++) {
                debug->verbose2("checking source %u -> %s\n",
                           i,
                           poll_if[i].revents & POLLIN ? "ready" : "no-data");
                time_t now;
                time(&now);

                if(poll_if[i].revents & POLLIN) {
                    all_if[i]->receive(now);
                    n--;
                }
            }
        } else {
            /* there was an error, maybe exited due to signal */
            perror("sunshine poll");
        }
	if(signal_usr2) {
	    things_to_do.printevents(debug->file, "usr2 ");
	    dag_network::print_all_dagstats(debug->file, "usr2 ");
	    signal_usr2 = false;
	}

    }
}

bool network_interface::faked(void) {
    return false;
};

/*
 * updates the checksum of a memory block at buff, length len,
 *
 * THIS seems BROKEN, (18-checksum) but it must be something outside.
 */
unsigned short network_interface::csum_partial(
    const unsigned char *buff, int len, unsigned sum)
{
    const unsigned short *data = (const unsigned short *)buff;

    while(len > 0) {
        sum = sum + *data;
        len -= 2;
        data++;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum = (sum & 0xFFFF) + (sum >> 16);
    return sum;
}

unsigned short network_interface::csum_ipv6_magic(
    const struct in6_addr *saddr,
    const struct in6_addr *daddr,
    __u32 len, unsigned short proto,
    unsigned sum)
{
    struct {
        struct in6_addr ph_src;
        struct in6_addr ph_dst;
        u_int32_t       ph_len;
        u_int8_t        ph_zero[3];
        u_int8_t        ph_nxt;
    } ph /*PACKED*/;

    /* pseudo-header */
    memset(&ph, 0, sizeof(ph));
    ph.ph_src = *saddr;
    ph.ph_dst = *daddr;
    ph.ph_len = htonl(len);
    ph.ph_nxt = proto;

#if 0
    printf("pseudo: %u\n", sizeof(ph));
    hexdump((unsigned char *)&ph, 0, sizeof(ph));
#endif

    sum = csum_partial((unsigned char *)&ph, sizeof(ph), sum);
    return sum;
}


void network_interface::send_dio_all(dag_network *dag)
{
    class network_interface *iface = all_if;

    /*
     * would be more efficient to move build_dio here. because we can
     * probably build the same DIO message for all interfaces.
     */
    while(iface != NULL) {
	if(iface->is_active()) {
	    iface->debug->log("iface %s sending dio about dag: %s\n",
                              iface->if_name, dag->get_dagName());
	    iface->send_dio(dag);
	}
	iface = iface->next;
    }
}

/* really, seldom used */
#if 0
void network_interface::send_dao_all(dag_network *dag)
{
    class network_interface *iface = all_if;
    while(iface != NULL) {
	iface->debug->log("iface %s sending dao\n", iface->if_name);
	if(iface->nd_socket != -1) {
            dag->send_dao();
	}
	iface = iface->next;
    }
}
#endif

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
