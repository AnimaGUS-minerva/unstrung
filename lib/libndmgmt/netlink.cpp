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
#include <arpa/inet.h>
#include <linux/if_arp.h>       /* for ARPHRD_ETHER */
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <sys/ioctl.h>
#include <time.h>
#include "oswlibs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <netlink/rt_names.h>
#include <netlink/utils.h>
#include <netlink/ll_map.h>
#include <linux/rtnetlink.h>

#include "rpl.h"
}

#include "iface.h"
#include "prefix.h"

#ifndef ARPHRD_IEEE802154
#define ARPHRD_IEEE802154 804
#endif
#ifndef ARPHRD_6LOWPAN
#define ARPHRD_6LOWPAN 825
#endif


struct rtnl_handle* network_interface::netlink_handle = NULL;

class iface_factory basic_factory;
class iface_factory *iface_maker = &basic_factory;

network_interface *iface_factory::newnetwork_interface(const char *name, rpl_debug *deb)
{
    return new network_interface(name, deb);
}

/* used by addprefix() to change system parameters */
int network_interface::nisystem(const char *cmd)
{
    ::system(cmd);

}

/* used by addprefix() to change system parameters */
int network_interface::ni_route_show(void)
{
    nisystem("ip -6 route show");
}

/* this is wrong, use netlink to set the address later on. */
bool network_interface::addprefix(dag_network *dn _U_,  prefix_node &prefix)
{
    char buf[1024];
    ip_subnet newipv6;
    const char *viaif = if_name;

    if(prefix.prefix_valid()) {
        // this would be better, but results in unreachable routes.
        viaif = "lo";
        snprintf(buf, 1024,
                 "ip -6 addr del %s dev %s", prefix.node_name(), viaif);
        debug->log("  invoking %s\n", buf);
        nisystem(buf);

        snprintf(buf, 1024,
                 "ip -6 addr add %s dev %s", prefix.node_name(), viaif);

        debug->log("  invoking %s\n", buf);
        nisystem(buf);
        ni_route_show();
    }

    return true;
}

/* XXX do this with netlink too  */
bool network_interface::add_parent_route_to_prefix(const ip_subnet &prefix,
                                                   const ip_address *src,
                                                   /*const*/rpl_node &parent)
{
    char buf[1024];
    char pbuf[SUBNETTOT_BUF];
    char nhbuf[ADDRTOT_BUF];
    const char *srcstring = "";
    char srcbuf[ADDRTOT_BUF];

    subnettot(&prefix, 0, pbuf, sizeof(pbuf));
    addrtot(&parent.node_address(), 0, nhbuf, sizeof(nhbuf));

    srcbuf[0]='\0';
    if(src) {
        addrtot(src, 0, srcbuf, sizeof(nhbuf));
        srcstring = "src ";
    }

    snprintf(buf, 1024, "ip -6 route del %s", pbuf);
    debug->log("  invoking %s\n", buf);
    nisystem(buf);

    snprintf(buf, 1024,
             "ip -6 route add %s via %s dev %s %s%s",
             pbuf,  nhbuf,
             this->get_if_name(),
             srcstring, srcbuf);


    debug->log("  invoking %s\n", buf);
    nisystem(buf);
    ni_route_show();
}

/* XXX do this with netlink too  */
bool network_interface::add_parent_route_to_default(const ip_address *src,
                                                    /*const*/rpl_node &parent)
{
    ip_subnet defaultroute;

    memset(&defaultroute, 0, sizeof(defaultroute));
    defaultroute.maskbits = 0;                     /* maybe should say 2000::/3 */
    defaultroute.addr.u.v6.sin6_family = AF_INET6;

    return add_parent_route_to_prefix(defaultroute, src, parent);
}


/* XXX do this with netlink too  */
bool network_interface::add_null_route_to_prefix(const ip_subnet &prefix)
{
    char buf[1024];
    char pbuf[SUBNETTOT_BUF];

    subnettot(&prefix, 0, pbuf, sizeof(pbuf));

    snprintf(buf, 1024,
             "ip -6 route add unreachable %s dev lo", pbuf);

    debug->log("  invoking %s\n", buf);
    nisystem(buf);
    ni_route_show();
}

/* XXX do this with netlink too  */
bool network_interface::add_route_to_node(const ip_subnet &prefix,
                                          rpl_node *peer,
                                          const ip_address &src)
{
    char buf[1024];

    char pbuf[SUBNETTOT_BUF], tbuf[SUBNETTOT_BUF];
    char sbuf[SUBNETTOT_BUF];

    prefix_node &n = ipv6_prefix_list[prefix];
    n.set_prefix(prefix);

    subnettot(&n.prefix_number(), 0, pbuf, sizeof(pbuf));
    addrtot(&peer->node_address(),  0, tbuf, sizeof(tbuf));
    addrtot(&src,                 0, sbuf, sizeof(sbuf));

    snprintf(buf, 1024,
             "ip -6 route replace %s via %s dev %s src %s", pbuf, tbuf, if_name, sbuf);

    debug->log("  invoking %s\n", buf);
    nisystem(buf);
    ni_route_show();

    return true;
}

/*
 * get the hatype, and save it
 *
 */
unsigned int network_interface::get_hatype(void)
{
    struct ifreq ifr0;
    struct sockaddr_ll me;
    socklen_t alen;
    int s;

    if(this->hatype != 0) return this->hatype;

    /* look it up the old way */

    s = socket(PF_PACKET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket(PF_PACKET)");
        return -1;
    }

    memset(&me, 0, sizeof(me));
    me.sll_family = AF_PACKET;
    me.sll_ifindex = this->if_index;
    me.sll_protocol = htons(ETH_P_LOOP);
    if (bind(s, (struct sockaddr*)&me, sizeof(me)) == -1) {
        perror("bind");
        close(s);
        return -1;
    }

    alen = sizeof(me);
    if (getsockname(s, (struct sockaddr*)&me, &alen) == -1) {
        perror("getsockname");
        close(s);
        return -1;
    }
    close(s);

    this->hatype = me.sll_hatype;
    return this->hatype;
}

/*
 * set the long and short link-layer-addresses addresses.
 *
 */
bool network_interface::set_link_layer64(const unsigned char eui64bytes[8],
                                         unsigned int eui64len)
{
    if(eui64len==6) {
        memcpy(eui48, eui64bytes, 6);
        eui64_from_eui48();
    } else if(eui64len==8) {
        memcpy(eui64, eui64bytes, 8);
        eui64set = true;
    } else {
        return false;
    }

    switch(this->ifi_type) {
    case ARPHRD_6LOWPAN:
        memcpy(this->if_hwaddr, eui64, 8);
        this->if_hwaddr_len = 8;
        break;
    default:
        memcpy(this->if_hwaddr, eui48, 6);
        this->if_hwaddr_len = 6;
        break;
    }
}

bool network_interface::set_link_layer64_hw(void)
{
    struct ifreq ifr0;
    int s;

    memset(&ifr0, 0, sizeof(ifr0));
    strncpy(ifr0.ifr_name, this->if_name, IFNAMSIZ);
    ifr0.ifr_hwaddr.sa_family = this->get_hatype();

    memcpy(ifr0.ifr_hwaddr.sa_data, this->if_hwaddr, this->if_hwaddr_len);

    s = socket(PF_PACKET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket(PF_PACKET)");
        return false;
    }

    if (ioctl(s, SIOCSIFHWADDR, &ifr0) < 0) {
        perror("SIOCSIFHWADDR");
        close(s);
        return false;
    }
    close(s);
    return true;
}

/*
 * implements:
 *      ip link add link wpan2 name lowpan0 type lowpan
 *
 * where will wpan2 and lowpan0 come from!!!
 *
 */
struct iplink_req {
    struct nlmsghdr		n;
    struct ifinfomsg	i;
    char			buf[1024];
};

int network_interface::configure_wpan(void)
{

    struct iplink_req req;
    char *dev  = this->if_name;
    char *name = this->if_name;
    const char *link = "wpan2";
    int group = 0;
    int index = this->if_index;
    const int cmd   = RTM_NEWLINK;
    const int flags = NLM_F_CREATE|NLM_F_EXCL;

    if(this->ifi_type != ARPHRD_6LOWPAN) {
        debug->info("no lowpan configuration needed for interface: %s\n", if_name);
        return true;
    } else {
        debug->info("linking new lowpan%u on top of %s\n", 0, if_name);
    }


    memset(&req, 0, sizeof(req));

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.n.nlmsg_flags = NLM_F_REQUEST|flags;
    req.n.nlmsg_type = RTM_NEWLINK;
    req.i.ifi_family = AF_PACKET;
    req.i.ifi_change |= IFF_UP;
    req.i.ifi_flags  |= IFF_UP;

    addattr_l(&req.n, sizeof(req), IFLA_GROUP,
              &group, sizeof(group));

    addattr_l(&req.n, sizeof(req), IFLA_LINK, &if_index, 4);

    unsigned int len = strlen(name);
    addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name, len);

    {
        struct rtattr *linkinfo;
        linkinfo = addattr_nest(&req.n, sizeof(req), IFLA_LINKINFO);

        const char *type = "lowpan";
        addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, type, strlen(type));
        addattr_nest_end(&req.n, linkinfo);
    }

    if (rtnl_talk(netlink_handle, &req.n, 0, 0, NULL, NULL, NULL) < 0)
        return false;

    return true;
}


int network_interface::gather_linkinfo(const struct sockaddr_nl *who,
                           struct nlmsghdr *n, void *arg)
{

    switch(n->nlmsg_type) {
    case RTM_NEWLINK:
        add_linkinfo(who, n, arg);
        break;
    case RTM_DELLINK:
        del_linkinfo(who, n, arg);
        break;
    case RTM_NEWADDR:
        adddel_ipinfo(who, n, arg);
        break;
    case RTM_DELADDR:
        /* handle this in some way */
        break;
    default:
        fprintf(stderr, "ignored nlmsgtype: %u\n", n->nlmsg_type);
    }
}

/* from netifd */
static char dev_buf[256];
static int system_get_sysctl(const char *path, char *buf, const size_t buf_sz)
{
	int fd = -1, ret = -1;

	fd = open(path, O_RDONLY);
        if(fd >= 0) {
            ssize_t len = read(fd, buf, buf_sz - 1);
            if (len >= 0) {
                ret = buf[len] = 0;
            }
        }

	if (fd >= 0)
		close(fd);

	return ret;
}

static int
system_get_dev_sysctl(const char *path, const char *device, char *buf, const size_t buf_sz)
{
	snprintf(dev_buf, sizeof(dev_buf), path, device);
	return system_get_sysctl(dev_buf, buf, buf_sz);
}

bool network_interface::system_get_disable_ipv6(void)
{
    char b1[64];
    system_get_dev_sysctl("/proc/sys/net/ipv6/conf/%s/disable_ipv6",
                          this->if_name, b1, sizeof(b1));
    bool disable = (strtoul(b1, NULL, 10) != 0);
    return disable;
}


int network_interface::adddel_ipinfo(const struct sockaddr_nl *who,
                                       struct nlmsghdr *n, void *arg)
{
    struct network_interface_init *nii = (struct network_interface_init *)arg;
    rpl_debug *deb = nii->debug;
    struct ifaddrmsg *iai = (struct ifaddrmsg *)NLMSG_DATA(n);
    struct rtattr * tb[IFA_MAX+1], *addrattr;
    int len = n->nlmsg_len;
    unsigned m_flag = 0;

    len -= NLMSG_LENGTH(sizeof(*iai));
    if (len < 0)
        return -1;

    parse_rtattr(tb, IFA_MAX, IFA_RTA(iai), len);
    addrattr = tb[IFA_LOCAL];
    if(addrattr == NULL) {
        addrattr = tb[IFA_ADDRESS];
    }

    if (addrattr == NULL) {
        /* not a useful update */
        return 0;
    }

    network_interface *ni = find_by_ifindex(iai->ifa_index);
    if(ni == NULL) {
        /*
         * might work if we have a name to go with it, but we will
         * not create it for now.
         */
        // deb->warn("Not creating new interface index=%d\n", iai->ifa_index);
        return 0;
    }

    const unsigned char *addr = NULL;
    unsigned int addrlen = 0;

    switch(iai->ifa_family) {
    case AF_INET6:
        addr = (unsigned char *)RTA_DATA(addrattr);
        if(addr) {
            addrlen = RTA_PAYLOAD(addrattr);
            if(addrlen > sizeof(ni->if_addr)) addrlen=sizeof(ni->if_addr);

            memcpy(&ni->if_addr, addr, addrlen);
        }
        ni->ifa_scope = iai->ifa_scope;
        ni->update_addr();
        break;

    default:
        break;
    }

    return 0;
}

void network_interface::update_addr(void)
{
    bool announced = false;
    SPRINT_BUF(b1);

    if(this->node) {
        delete this->node;
    }
    this->node = new rpl_node(if_addr);
    this->node->debug = debug;

    inet_ntop(AF_INET6, &if_addr, b1, sizeof(b1));

    /* scopes are reversed on Linux: HOST=254, LINK=253, UNIVERSE=0 */
    if(ifa_scope <= RT_SCOPE_LINK) {
        /* now see if this IP address should be added to future DAOs */
        announced = dag_network::notify_new_interface(this);
    }

    /* log it for human */
    debug->info("ip found[%d]: %s scope=%u address=%s%s\n",
              this->if_index, this->if_name, ifa_scope,
              b1, announced ? " announced" : "");
}


int network_interface::del_linkinfo(const struct sockaddr_nl *who,
                                       struct nlmsghdr *n, void *arg)
{
    struct network_interface_init *nii = (struct network_interface_init *)arg;
    rpl_debug *deb = nii->debug;
    struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(n);
    FILE *fp = stdout;
    struct rtattr * tb[IFLA_MAX+1];
    int len = n->nlmsg_len;
    unsigned m_flag = 0;
    SPRINT_BUF(b1);

    len -= NLMSG_LENGTH(sizeof(*ifi));
    if (len < 0)
        return -1;

    parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
    if (tb[IFLA_IFNAME] == NULL) {
        fprintf(stderr, "BUG: nil ifname\n");
        return -1;
    }

    network_interface *ni = find_by_ifindex(ifi->ifi_index);
    const char *ifname = (const char*)RTA_DATA(tb[IFLA_IFNAME]);

    if(ni == NULL) {
        deb->info("link deleted[%d]: %s type=%s [ignored]\n",
                  ni->if_index, ifname,
                  ll_type_n2a(ifi->ifi_type, b1, sizeof(b1)));
        return 0;
    }

    deb->info("link deleted[%d]: %s type=%s (%s %s)%s\n",
              ni->if_index, ifname,
              ll_type_n2a(ifi->ifi_type, b1, sizeof(b1)),
              ni->alive   ? "active" : "inactive",
              ni->on_list ? "existing" :"new",
              ni->faked() ? " faked" : "");
    delete ni;
    return 0;
}

int network_interface::add_linkinfo(const struct sockaddr_nl *who,
                                       struct nlmsghdr *n, void *arg)
{
    struct network_interface_init *nii = (struct network_interface_init *)arg;
    rpl_debug *deb = nii->debug;
    struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(n);
    FILE *fp = stdout;
    struct rtattr * tb[IFLA_MAX+1];
    int len = n->nlmsg_len;
    unsigned m_flag = 0;
    SPRINT_BUF(b1);

    len -= NLMSG_LENGTH(sizeof(*ifi));
    if (len < 0)
        return -1;

    parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
    if (tb[IFLA_IFNAME] == NULL) {
        deb->error("BUG: nil ifname\n");
        return -1;
    }

    network_interface *ni = find_by_ifindex(ifi->ifi_index);
    const char *ifname = (const char*)RTA_DATA(tb[IFLA_IFNAME]);

    if(ni == NULL) {
        ni = iface_maker->newnetwork_interface(ifname, deb);
        ni->if_index = ifi->ifi_index;
        ni->set_debug(deb);
    }

    /* log it for human */
    deb->info("link found[%d]: %s type=%s (%s %s)%s\n",
             ni->if_index, ni->get_if_name(),
             ll_type_n2a(ifi->ifi_type, b1, sizeof(b1)),
             ni->alive   ? "active" : "inactive",
             ni->on_list ? "existing" :"new",
             ni->faked() ? " faked" : "");

    ni->add_to_list();

    /* dummy0 interface is always changing */
    if(strcasecmp(ni->if_name, "dummy0")==0) {
        return 0;
    }

    if (ni->system_get_disable_ipv6()) {
        /* IPv6 is disabled, ignore this interface */
        deb->warn("interface %s has IPv6 disabled; ignored\n", ni->if_name);
        ni->disabled = true;
        return 0;
    }

    const unsigned char *addr = NULL;
    unsigned int addrlen = 0;

    ni->ifi_type = ifi->ifi_type;
    switch(ifi->ifi_type) {
    case ARPHRD_ETHER:
        addr = (unsigned char *)RTA_DATA(tb[IFLA_ADDRESS]);
        if(addr) {
            addrlen = RTA_PAYLOAD(tb[IFLA_ADDRESS]);
            ni->set_eui48(addr, addrlen);
            break;
        }

    case ARPHRD_IEEE802154:
    case ARPHRD_6LOWPAN:
        addr = (unsigned char *)RTA_DATA(tb[IFLA_ADDRESS]);
        if(addr) {
            addrlen = RTA_PAYLOAD(tb[IFLA_ADDRESS]);
            ni->set_eui64(addr, addrlen);
            break;
        }

    case ARPHRD_LOOPBACK:
        loopback_interface = ni;
        ni->loopback = true;
        return 0;

    default:
        deb->log("   ignoring address type: %d\n", ifi->ifi_type);
        return 0;
    }

    if (tb[IFLA_MTU]) {
        ni->if_maxmtu =  *(int*)RTA_DATA(tb[IFLA_MTU]);
    }

    if (!ni->eui64set){
    	/* must be a new mac address */
    	memcpy(ni->eui48, addr, addrlen);

    	/* now build eui64 from eui48 */
    	ni->generate_eui64();
    }

    SPRINT_BUF(b2);
    deb->log("   adding as new interface %s/%s\n",
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

        if(rtnl_open(netlink_handle, RTMGRP_LINK|RTMGRP_IPV6_IFADDR) < 0) {
            fprintf(stderr, "Cannot open rtnetlink!!\n");
            free(netlink_handle);
            netlink_handle = NULL;
            return false;
        }
    }
    return true;
}

void network_interface::empty_socket(rpl_debug *deb)
{
    struct network_interface_init nii;
    nii.debug = deb;
    nii.setup = true;

    int results = rtnl_listen(netlink_handle, gather_linkinfo, (void *)&nii);
    if(results != -1 && results != 0) {
        deb->info("empty_socket returned with %d\n", results);
    }
}

int network_interface::setup_msg_callback(rpl_debug *deb)
{
    int fd = rtnl_socket_get_fd(netlink_handle);

    /* make it non-blocking */
    if(fcntl(fd, F_SETFL, O_NONBLOCK) != 0) {
        deb->error("can not set non-blocking mode on netlink socket: %s\n",
                   strerror(errno));
        return -1;
    }

    /*
     * now subscribe to netfilter events to get updates to routing
     * and address changes
     */
#if 0
    nl_socket_modify_cb(netlink_handle,
                        NL_CB_VALID, NL_CB_CUSTOM,
                        unstrung_update_msg_parser, nii);
#endif
    return fd;
}



bool network_interface::scan_devices(rpl_debug *deb, bool setup)
{
	struct nlmsg_list *linfo = NULL;
	struct nlmsg_list *ainfo = NULL;
	struct nlmsg_list *l, *n;
	char *filter_dev = NULL;
	int no_link = 0;
        struct network_interface_init nii;

        if(!open_netlink()) return true;

        remove_marks();

        nii.debug = deb;
        nii.setup = setup;

        /* get list of interfaces */
	if (rtnl_wilddump_request(netlink_handle, AF_PACKET, RTM_GETLINK) < 0) {
            deb->error("Cannot send dump request");
            return false;
	}

	if (rtnl_dump_filter(netlink_handle, gather_linkinfo,
                             (void *)&nii, NULL, NULL) < 0) {
            deb->error("Dump terminated\n");
            return false;
	}

        /* get list of addresses on the interfaces */
	if (rtnl_wilddump_request(netlink_handle, AF_INET6, RTM_GETADDR) < 0) {
            deb->error("Cannot send dump request: %s", strerror(errno));

	}

	if (rtnl_dump_filter(netlink_handle, gather_linkinfo,
                             (void *)&nii, NULL, NULL) < 0) {
            deb->error("Dump terminated\n");
            return false;
	}

        /* now look for interfaces with no mark, as they may be removed */
        /* XXX */
        return true;
}

/*
 * determines if an interface is a 6lowpan interface, and if so, sets
 * the eui64.
 */
bool network_interface::setup_lowpan(const unsigned char eui64[8],
                                    unsigned int eui64len)
{
    unsigned int ha1 = this->get_hatype();

    char eui64buf[128];
    network_interface::fmt_eui(eui64buf, sizeof(eui64buf), eui64, eui64len);

    debug->info("assigning %s to interface: %s %u\n", eui64buf, this->if_name, ha1);

    this->set_link_layer64(eui64, eui64len);
    this->set_link_layer64_hw();
    this->configure_wpan();
}



/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
