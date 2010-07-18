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
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include <time.h>

#include <netlink/rt_names.h>
#include <netlink/utils.h>
#include <netlink/ll_map.h>


#include "rpl.h"
}

#include "iface.h"
#include "prefix.h"

struct rtnl_handle strung_rth;


/* this is wrong, use netlink to set the address later on. */
bool network_interface::addprefix(prefix_node &prefix)
{
        char buf[1024];
        ip_subnet newipv6;

        newipv6 = prefix.get_prefix();
        newipv6.maskbits = 128;
        memcpy(&newipv6.addr.u.v6.sin6_addr.s6_addr[8],
               eui64, 8);

        snprintf(buf, 1024,
                 "ip -6 addr add XX");
        //system(buf);
}

static struct
{
	int ifindex;
	int family;
	int oneline;
	int showqueue;
//	inet_prefix pfx;
	int scope, scopemask;
	int flags, flagmask;
	int up;
	char *label;
	int flushed;
	char *flushb;
	int flushp;
	int flushe;
} filter;

void print_link_flags(FILE *fp, unsigned flags, unsigned mdown)
{
	fprintf(fp, "<");
	if (flags & IFF_UP && !(flags & IFF_RUNNING))
		fprintf(fp, "NO-CARRIER%s", flags ? "," : "");
	flags &= ~IFF_RUNNING;
#define _PF(f) if (flags&IFF_##f) { \
                  flags &= ~IFF_##f ; \
                  fprintf(fp, #f "%s", flags ? "," : ""); }
	_PF(LOOPBACK);
	_PF(BROADCAST);
	_PF(POINTOPOINT);
	_PF(MULTICAST);
	_PF(NOARP);
	_PF(ALLMULTI);
	_PF(PROMISC);
	_PF(MASTER);
	_PF(SLAVE);
	_PF(DEBUG);
	_PF(DYNAMIC);
	_PF(AUTOMEDIA);
	_PF(PORTSEL);
	_PF(NOTRAILERS);
	_PF(UP);
#undef _PF
        if (flags)
		fprintf(fp, "%x", flags);
	if (mdown)
		fprintf(fp, ",M-DOWN");
	fprintf(fp, "> ");
}

int print_linkinfo(const struct sockaddr_nl *who, 
		   struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE*)arg;
	struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(n);
	struct rtattr * tb[IFLA_MAX+1];
	int len = n->nlmsg_len;
	unsigned m_flag = 0;

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

	if (filter.ifindex && ifi->ifi_index != filter.ifindex)
		return 0;
	if (filter.up && !(ifi->ifi_flags&IFF_UP))
		return 0;

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
	if (tb[IFLA_IFNAME] == NULL) {
		fprintf(stderr, "BUG: nil ifname\n");
		return -1;
	}
#if 0
	if (filter.label &&
	    (!filter.family || filter.family == AF_PACKET) &&
	    fnmatch(filter.label, RTA_DATA(tb[IFLA_IFNAME]), 0))
		return 0;
#endif

	if (n->nlmsg_type == RTM_DELLINK)
		fprintf(fp, "Deleted ");

	fprintf(fp, "%d: %s", ifi->ifi_index,
		tb[IFLA_IFNAME] ? (char*)RTA_DATA(tb[IFLA_IFNAME]) : "<nil>");

	if (tb[IFLA_LINK]) {
		SPRINT_BUF(b1);
		int iflink = *(int*)RTA_DATA(tb[IFLA_LINK]);
		if (iflink == 0)
			fprintf(fp, "@NONE: ");
		else {
			fprintf(fp, "@%s: ", ll_idx_n2a(iflink, b1));
			m_flag = ll_index_to_flags(iflink);
			m_flag = !(m_flag & IFF_UP);
		}
	} else {
		fprintf(fp, ": ");
	}
	print_link_flags(fp, ifi->ifi_flags, m_flag);

	if (tb[IFLA_MTU])
		fprintf(fp, "mtu %u ", *(int*)RTA_DATA(tb[IFLA_MTU]));
	if (tb[IFLA_QDISC])
		fprintf(fp, "qdisc %s ", (char*)RTA_DATA(tb[IFLA_QDISC]));
#ifdef IFLA_MASTER
	if (tb[IFLA_MASTER]) {
		SPRINT_BUF(b1);
		fprintf(fp, "master %s ", ll_idx_n2a(*(int*)RTA_DATA(tb[IFLA_MASTER]), b1));
	}
#endif
	
	if (!filter.family || filter.family == AF_PACKET) {
		SPRINT_BUF(b1);
		fprintf(fp, "%s", _SL_);
		fprintf(fp, "    link/%s ", ll_type_n2a(ifi->ifi_type, b1, sizeof(b1)));

		if (tb[IFLA_ADDRESS]) {
			fprintf(fp, "%s", (unsigned char *)
                                ll_addr_n2a((unsigned char *)RTA_DATA(tb[IFLA_ADDRESS]),
                                            RTA_PAYLOAD(tb[IFLA_ADDRESS]),
                                            ifi->ifi_type,
                                            b1, sizeof(b1)));
		}
		if (tb[IFLA_BROADCAST]) {
			if (ifi->ifi_flags&IFF_POINTOPOINT)
				fprintf(fp, " peer ");
			else
				fprintf(fp, " brd ");
			fprintf(fp, "%s",
                                ll_addr_n2a((unsigned char *)RTA_DATA(tb[IFLA_BROADCAST]),
						      RTA_PAYLOAD(tb[IFLA_BROADCAST]),
						      ifi->ifi_type,
						      b1, sizeof(b1)));
		}
	}

#if 0
	if (tb[IFLA_STATS] && show_stats) {
		struct rtnl_link_stats slocal;
		struct rtnl_link_stats *s = (struct rtnl_link_stats *)RTA_DATA(tb[IFLA_STATS]);
		if (((unsigned long)s) & (sizeof(unsigned long)-1)) {
			memcpy(&slocal, s, sizeof(slocal));
			s = &slocal;
		}
		fprintf(fp, "%s", _SL_);
		fprintf(fp, "    RX: bytes  packets  errors  dropped overrun mcast   %s%s",
			s->rx_compressed ? "compressed" : "", _SL_);
		fprintf(fp, "    %-10u %-8u %-7u %-7u %-7u %-7u",
			s->rx_bytes, s->rx_packets, s->rx_errors,
			s->rx_dropped, s->rx_over_errors,
			s->multicast
			);
		if (s->rx_compressed)
			fprintf(fp, " %-7u", s->rx_compressed);
		if (show_stats > 1) {
			fprintf(fp, "%s", _SL_);
			fprintf(fp, "    RX errors: length  crc     frame   fifo    missed%s", _SL_);
			fprintf(fp, "               %-7u  %-7u %-7u %-7u %-7u",
				s->rx_length_errors,
				s->rx_crc_errors,
				s->rx_frame_errors,
				s->rx_fifo_errors,
				s->rx_missed_errors
				);
		}
		fprintf(fp, "%s", _SL_);
		fprintf(fp, "    TX: bytes  packets  errors  dropped carrier collsns %s%s",
			s->tx_compressed ? "compressed" : "", _SL_);
		fprintf(fp, "    %-10u %-8u %-7u %-7u %-7u %-7u",
			s->tx_bytes, s->tx_packets, s->tx_errors,
			s->tx_dropped, s->tx_carrier_errors, s->collisions);
		if (s->tx_compressed)
			fprintf(fp, " %-7u", s->tx_compressed);
		if (show_stats > 1) {
			fprintf(fp, "%s", _SL_);
			fprintf(fp, "    TX errors: aborted fifo    window  heartbeat%s", _SL_);
			fprintf(fp, "               %-7u  %-7u %-7u %-7u",
				s->tx_aborted_errors,
				s->tx_fifo_errors,
				s->tx_window_errors,
				s->tx_heartbeat_errors
				);
		}
	}
#endif
	fprintf(fp, "\n");
	fflush(fp);
	return 0;
}

void ipaddr_reset_filter(int oneline)
{
	memset(&filter, 0, sizeof(filter));
	filter.oneline = oneline;
}

struct nlmsg_list
{
	struct nlmsg_list *next;
	struct nlmsghdr	  h;
};

static int store_nlmsg(const struct sockaddr_nl *who, struct nlmsghdr *n, 
		       void *arg)
{
	struct nlmsg_list **linfo = (struct nlmsg_list**)arg;
	struct nlmsg_list *h;
	struct nlmsg_list **lp;

	h = (struct nlmsg_list *)malloc(n->nlmsg_len+sizeof(void*));
	if (h == NULL)
		return -1;

	memcpy(&h->h, n, n->nlmsg_len);
	h->next = NULL;

	for (lp = linfo; *lp; lp = &(*lp)->next) /* NOTHING */;
	*lp = h;

	ll_remember_index(who, n, NULL);
	return 0;
}

void network_interface::find_eui64(void)
{
	struct nlmsg_list *linfo = NULL;
	struct nlmsg_list *ainfo = NULL;
	struct nlmsg_list *l, *n;
	char *filter_dev = NULL;
	int no_link = 0;

	ipaddr_reset_filter(/* oneline */0);
	filter.showqueue = 1;

        filter.ifindex = if_index;
        filter.family  = AF_PACKET;

	if (rtnl_wilddump_request(&strung_rth, filter.family, RTM_GETLINK) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	if (rtnl_dump_filter(&strung_rth, store_nlmsg, &linfo, NULL, NULL) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}

#if 0
	if (filter.family != AF_PACKET) {
		if (rtnl_wilddump_request(&strung_rth, filter.family, RTM_GETADDR) < 0) {
			perror("Cannot send dump request");
			exit(1);
		}

		if (rtnl_dump_filter(&strung_rth, store_nlmsg, &ainfo, NULL, NULL) < 0) {
			fprintf(stderr, "Dump terminated\n");
			exit(1);
		}
	}

	if (filter.family && filter.family != AF_PACKET) {
		struct nlmsg_list **lp;
		lp=&linfo;

		if (filter.oneline)
			no_link = 1;

		while ((l=*lp)!=NULL) {
			int ok = 0;
			struct ifinfomsg *ifi = NLMSG_DATA(&l->h);
			struct nlmsg_list *a;

			for (a=ainfo; a; a=a->next) {
				struct nlmsghdr *n = &a->h;
				struct ifaddrmsg *ifa = NLMSG_DATA(n);

				if (ifa->ifa_index != ifi->ifi_index || 
				    (filter.family && filter.family != ifa->ifa_family))
					continue;
				if ((filter.scope^ifa->ifa_scope)&filter.scopemask)
					continue;
				if ((filter.flags^ifa->ifa_flags)&filter.flagmask)
					continue;
				if (filter.pfx.family || filter.label) {
					struct rtattr *tb[IFA_MAX+1];
					parse_rtattr(tb, IFA_MAX, IFA_RTA(ifa), IFA_PAYLOAD(n));
					if (!tb[IFA_LOCAL])
						tb[IFA_LOCAL] = tb[IFA_ADDRESS];

					if (filter.pfx.family && tb[IFA_LOCAL]) {
						inet_prefix dst;
						memset(&dst, 0, sizeof(dst));
						dst.family = ifa->ifa_family;
						memcpy(&dst.data, RTA_DATA(tb[IFA_LOCAL]), RTA_PAYLOAD(tb[IFA_LOCAL]));
						if (inet_addr_match(&dst, &filter.pfx, filter.pfx.bitlen))
							continue;
					}
					if (filter.label) {
						SPRINT_BUF(b1);
						const char *label;
						if (tb[IFA_LABEL])
							label = RTA_DATA(tb[IFA_LABEL]);
						else
							label = ll_idx_n2a(ifa->ifa_index, b1);
						if (fnmatch(filter.label, label, 0) != 0)
							continue;
					}
				}

				ok = 1;
				break;
			}
			if (!ok)
				*lp = l->next;
			else
				lp = &l->next;
		}
	}
#endif

	for (l=linfo; l; l = n) {
		n = l->next;
		if (no_link || print_linkinfo(NULL, &l->h, stdout) == 0) {
			struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(&l->h);
#if 0
			if (filter.family != AF_PACKET)
				print_selected_addrinfo(ifi->ifi_index, ainfo, stdout);
#endif
		}
		fflush(stdout);
		free(l);
	}
}
