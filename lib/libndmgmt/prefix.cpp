/*
 * Copyright (C) 2010-2013 Michael Richardson <mcr@sandelman.ca>
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

extern "C" {
#include <arpa/inet.h>
#include <stdio.h>
#include <stdarg.h>
#include "oswlibs.h"
};

#include "dag.h"
#include "node.h"
#include "prefix.h"
#include "iface.h"

prefix_node::prefix_node(rpl_debug *deb, rpl_node *announcer, ip_subnet sub)
{
    name[0]='\0';
    mPrefix = sub;
    announced_from = announcer;
    valid = true;
    installed = false;
    debug = deb;
}

prefix_node::prefix_node(rpl_debug *deb, const struct in6_addr v6, const int prefixlen)
{
    debug = deb;
    name[0]='\0';
    set_prefix(v6, prefixlen);
}

void prefix_node::set_prefix(const struct in6_addr v6, const int prefixlen)
{
    memset(&mPrefix, 0, sizeof(mPrefix));
    mPrefix.addr.u.v6.sin6_family = AF_INET6;
    mPrefix.addr.u.v6.sin6_flowinfo = 0;		/* unused */
    mPrefix.addr.u.v6.sin6_port = 0;
    memcpy((void *)&mPrefix.addr.u.v6.sin6_addr, (void *)&v6, 16);
    mPrefix.maskbits  = prefixlen;
    name[0]='\0';
    valid = true;
    installed = false;
}

void prefix_node::set_prefix(ip_subnet prefix)
{
    mPrefix = prefix;
    name[0]='\0';
    //this->verbose_log("setting prefix, invalidating name");
    valid = true;
    installed = false;
}

const char *prefix_node::node_name() {
    if(valid) {
        if(name[0]) return name;

        subnettot(&mPrefix, 0, name, sizeof(name));
        //this->verbose_log("formatting prefix: %s", name);
        return name;
    } else {
        return "<prefix-not-valid>";
    }
};

void prefix_node::configureip(network_interface *iface, dag_network *dn)
{
    if(dn->mIgnorePio) {
        this->verbose_log("PIO ignored");
        return;
    }

    this->verbose_log("  peer '%s' announces prefix: %s\n",
                      announced_from ? announced_from->node_name() : "<none>",
                      dn->prefix_name());

    if(!installed) {
        struct in6_addr link;
        if(dn->mIID_is_set) {
            link = dn->mIID.u.v6.sin6_addr;
        } else {
            link = iface->link_local();
        }

        if(dn->myDeviceIdentity) {
            link = dn->myDeviceIdentity->sn.addr.u.v6.sin6_addr;
        } else {
            /* set upper 64-bits to prefix announced */
            /* was using memcpy, but taking address of rvalue no longer allowed */
            for(int i=0; i<8; i++) {
                link.s6_addr[i] = dn->get_prefix().addr.u.v6.sin6_addr.s6_addr[i];
            }
        }

        /* when configuring an IP address on the lo, it should be /128 */
        this->set_prefix(link, 128);
        //this->set_prefix(link, dn->get_prefix().maskbits);

        this->verbose_log("  adding prefix: %s learnt from iface: %s\n",
                          node_name(),
                          iface->get_if_name());


        if(iface->addprefix(dn, *this)) {
            installed = true;
        }
    }
}

void prefix_node::markself(dag_network *dn, ip_subnet prefix)
{
    /* addresses (such as link layer) found on interface at boot are already installed */
    installed = true;
    set_debug(dn->debug);
    set_dn(dn);
    set_prefix(prefix);
}

void prefix_node::add_route_via_node(network_interface *iface)
{
    //debug->info("  prefix %p installed: %u from: %p\n", this, this->installed, this->announced_from);
    if(!this->installed) {
        iface->add_route_to_node(this->mPrefix, this->announced_from,
                                 mDN->dag_me->prefix_number().addr);
        this->installed = true;
    }
}



/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */






