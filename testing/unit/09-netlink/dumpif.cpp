#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <assert.h>

#include "netlink/libnetlink.h"
#include "netlink/ll_map.h"
#include "netlink/rt_names.h"
}

#include "iface.h"
#include "dag.h"

/*
 * this program needs to dump the list of interfaces via netlink,
 * printing the MAC address of each interface that it sees.
 */

struct rtnl_handle my_rth;
const char * _SL_ = "dumpif";

static void print_link_flags(FILE *fp, unsigned int flags, unsigned int mdown)
{
	if (flags & IFF_UP && !(flags & IFF_RUNNING))
            fprintf(fp, flags ? "%s," : "%s", "NO-CARRIER");
	flags &= ~IFF_RUNNING;
#define _PF(f) if (flags&IFF_##f) {					\
		flags &= ~IFF_##f ;					\
		fprintf(fp, flags ? "%s," : "%s", #f); }
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
	_PF(LOWER_UP);
	_PF(DORMANT);
	_PF(ECHO);
#undef _PF
	if (flags)
            fprintf(fp, "%x", flags);
	if (mdown)
            fprintf(fp, ",%s", "M-DOWN");
}

int my_print_linkinfo(const struct sockaddr_nl *who,
                      struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE*)arg;
	struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(n);
	struct rtattr * tb[IFLA_MAX+1];
	int len = n->nlmsg_len;
	unsigned m_flag = 0;
	char b1[1024];

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
	if (tb[IFLA_IFNAME] == NULL) {
		fprintf(stderr, "BUG: nil ifname\n");
		return -1;
	}
	if (n->nlmsg_type == RTM_DELLINK)
		fprintf(fp, "Deleted ");

	fprintf(fp, "%d: %s", ifi->ifi_index,
		tb[IFLA_IFNAME] ? (char*)RTA_DATA(tb[IFLA_IFNAME]) : "<nil>");

	if (tb[IFLA_LINK]) {
		int iflink = *(int*)RTA_DATA(tb[IFLA_LINK]);
		if (iflink == 0)
			fprintf(fp, "@NONE: ");
		else {
                    fprintf(fp, "@%s: ", (char *)ll_idx_n2a(iflink, b1));
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
		fprintf(fp, "master %s ", ll_idx_n2a(*(int*)RTA_DATA(tb[IFLA_MASTER]), b1));
	}
#endif
        //print_queuelen((char*)RTA_DATA(tb[IFLA_IFNAME]));

        {
		fprintf(fp, "%s", _SL_);
		fprintf(fp, "    link/%s ", ll_type_n2a(ifi->ifi_type, b1, sizeof(b1)));

		if (tb[IFLA_ADDRESS]) {
                    ll_addr_n2a((unsigned char *)RTA_DATA(tb[IFLA_ADDRESS]),
                                RTA_PAYLOAD(tb[IFLA_ADDRESS]),
                                ifi->ifi_type,
                                b1, sizeof(b1));
                    fprintf(fp, "%s", b1);
		}
		if (tb[IFLA_BROADCAST]) {
			if (ifi->ifi_flags&IFF_POINTOPOINT)
				fprintf(fp, " peer ");
			else
				fprintf(fp, " brd ");
			ll_addr_n2a((unsigned char *)RTA_DATA(tb[IFLA_BROADCAST]),
                                    RTA_PAYLOAD(tb[IFLA_BROADCAST]),
                                    ifi->ifi_type,
                                    b1, sizeof(b1));
			fprintf(fp, "%s", b1);
		}
	}
	if (tb[IFLA_STATS]) {
		struct rtnl_link_stats slocal;
		struct rtnl_link_stats *s = RTA_DATA(tb[IFLA_STATS]);
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
	fprintf(fp, "\n");
	fflush(fp);
	return 0;
}

int main(int argc, char *argv[])
{
    int i;
    int preferred_family = AF_PACKET;

    if (rtnl_open(&my_rth, 0) < 0) {
            fprintf(stderr, "Cannot open rtnetlink\n");
            return -1;
    }

    if (rtnl_wilddump_request(&my_rth, preferred_family, RTM_GETLINK) < 0) {
        perror("Cannot send dump request");
        exit(1);
    }

    if (rtnl_dump_filter(&my_rth, my_print_linkinfo, stdout, NULL, NULL) < 0) {
        fprintf(stderr, "Dump terminated\n");
        exit(1);
    }

    exit(0);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */


