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
#include <net/if_arp.h>         /* for ARPHRD_ETHER */
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
    memcpy(&newipv6.addr.u.v6.sin6_addr.s6_addr[8], eui64, 8);

    char sbuf[SUBNETTOT_BUF];
    subnettot(&newipv6, 0, sbuf, sizeof(sbuf));
    
    snprintf(buf, 1024,
             "ip -6 addr add %s dev %s", sbuf, if_name);
    if(VERBOSE(this)) fprintf(this->verbose_file, "  invoking %s\n", buf);
    //system(buf);
    //system("ip -6 addr show");
}

int network_interface::gather_linkinfo(const struct sockaddr_nl *who, 
                           struct nlmsghdr *n, void *arg)
{
    struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(n);
    FILE *fp = stdout;
    struct rtattr * tb[IFLA_MAX+1];
    int len = n->nlmsg_len;
    unsigned m_flag = 0;
    SPRINT_BUF(b1);

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

    network_interface *ni = find_by_ifindex(ifi->ifi_index);
    if(ni == NULL) {
        ni = new network_interface((const char*)RTA_DATA(tb[IFLA_IFNAME]));
        ni->if_index = ifi->ifi_index;
    }

    /* XXX need to use logging interface */
    fprintf(stderr, "found[%d]: %s type=%s (%s %s)\n",
            ni->if_index, ni->if_name,
            ll_type_n2a(ifi->ifi_type, b1, sizeof(b1)),            
            ni->alive   ? "alive" : "inactive",
            ni->on_list ? "existing" :"new");

    ni->add_to_list();

    const unsigned char *addr = NULL;
    unsigned int addrlen = 0;

    switch(ifi->ifi_type) {
    case ARPHRD_ETHER:
        addr = (unsigned char *)RTA_DATA(tb[IFLA_ADDRESS]);
        if(addr) {
            addrlen = RTA_PAYLOAD(tb[IFLA_ADDRESS]);
            if(memcpy(ni->eui48, addr, addrlen)==0) {
                /* no change, go on to next interface */
                return 0;
            }
            break;
        }

    default:
        return 0;
    }

    if (tb[IFLA_MTU]) {
        ni->if_maxmtu =  *(int*)RTA_DATA(tb[IFLA_MTU]);
    }

    /* must be a new mac address */
    memcpy(ni->eui48, addr, addrlen);

    /* now build eui64 from eui48 */
    ni->generate_eui64();

    SPRINT_BUF(b2);
    fprintf(stderr, "   adding as new interface %s/%s\n",
            ni->eui48_str(b1,sizeof(b1)),
            ni->eui64_str(b2,sizeof(b2)));
            
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

        remove_marks();

	if (rtnl_wilddump_request(netlink_handle, AF_PACKET, RTM_GETLINK) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	if (rtnl_dump_filter(netlink_handle, gather_linkinfo,
                             NULL, NULL, NULL) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}

        /* now look for interfaces with no mark, as they may be removed */
        
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
