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

struct rtnl_handle* network_interface::netlink_handle = NULL;

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

int network_interface::gather_linkinfo(const struct sockaddr_nl *who, 
                           struct nlmsghdr *n, void *arg)
{
    struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(n);
    FILE *fp = stdout;
    struct rtattr * tb[IFLA_MAX+1];
    int len = n->nlmsg_len;
    unsigned m_flag = 0;

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
    //print_link_flags(fp, ifi->ifi_flags, m_flag);

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
	
            SPRINT_BUF(b1);
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
        
	fprintf(fp, "\n");
	fflush(fp);
	return 0;
}

bool network_interface::open_netlink()
{
    if(netlink_handle == NULL) {
        netlink_handle = (struct rtnl_handle*)malloc(sizeof(struct rtnl_handle));
        if(netlink_handle == NULL) {
            return false;
        }

        if(rtnl_open(netlink_handle, 0) < 0) {
            fprintf(stderr, "Cannot open rtnetlink!!\n");
            free(netlink_handle);
            netlink_handle = NULL;
            return false;
        }
    }
    return true;
}


void network_interface::scan_devices(void)
{
	struct nlmsg_list *linfo = NULL;
	struct nlmsg_list *ainfo = NULL;
	struct nlmsg_list *l, *n;
	char *filter_dev = NULL;
	int no_link = 0;

        if(!open_netlink()) return;

	if (rtnl_wilddump_request(netlink_handle, AF_PACKET, RTM_GETLINK) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	if (rtnl_dump_filter(netlink_handle, gather_linkinfo,
                             NULL, NULL, NULL) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
