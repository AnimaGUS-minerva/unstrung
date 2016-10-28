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
};

#include "node.h"
#include "iface.h"
#include "dag.h"
#include "grasp.h"

void rpl_node::set_addr(const char *ipv6) {
    if(inet_pton(AF_INET6, ipv6, &nodeip.u.v6.sin6_addr) == 1) {
        nodeip.u.v6.sin6_family = AF_INET6;
    }
    calc_name();
}

rpl_node::rpl_node(const char *ipv6,
                   dag_network *dn, rpl_debug *deb) {
    self = false;
    set_addr(ipv6);
    set_dag(dn, deb);
}

void rpl_node::set_addr(const struct in6_addr v6) {
    nodeip.u.v6.sin6_addr = v6;
    nodeip.u.v6.sin6_family=AF_INET6;
    calc_name();
}

rpl_node::rpl_node(const struct in6_addr v6) {
    valid = true;
    self  = false;
    set_addr(v6);
}
rpl_node::rpl_node(const struct in6_addr v6,
                   dag_network *dn, rpl_debug *deb) {
    self = false;
    set_addr(v6);
    set_dag(dn, deb);
}

void rpl_node::calc_name(void) {
    char *addr = name;
    if(self) {
        strcpy(addr, "<ME>");
        addr += 4;
    }

    inet_ntop(AF_INET6, &nodeip.u.v6.sin6_addr, addr, INET6_ADDRSTRLEN-(addr-name));
}

const char *rpl_node::node_name() {
    if(valid) {
        if(name[0]) return name;

        calc_name();

        return name;
    } else {
        return "<node-not-valid>";
    }
};

char *rpl_node::fmt_eui64(const unsigned char eui64[8], char *buf, size_t buf_len)
{
    snprintf(buf, buf_len, "%02x%02x:%02x%02x:%02x%02x:%02x%02x",
             eui64[0], eui64[1], eui64[2], eui64[3],
             eui64[4], eui64[5], eui64[6], eui64[7]);
    return buf;
}

void rpl_node::set_dag(dag_network *dn,
                       rpl_debug *deb)
{
    if(dn)  mDN = dn;
    if(deb) this->debug = deb;
    couldBeValid();
}

void rpl_node::couldBeValid(void)
{
    if(!valid) {
        if(mDN != NULL
           && this->debug != NULL
           && nodeip.u.v6.sin6_family == AF_INET6) {
            valid  = true;
            debug->verbose("  new RPL node: %s\n", node_name());
        }
    }
}

void rpl_node::makevalid(const struct in6_addr v6,
                         dag_network *dn,
                         rpl_debug *deb)
{
    if(!valid) {
        nodeip.u.v6.sin6_addr = v6;
	nodeip.u.v6.sin6_family=AF_INET6;
        mDN    = dn;
        this->debug  = deb;
        couldBeValid();
    }
}


void rpl_node::update_nce_stamp(void)
{
    time(&this->lastseen);
}

void rpl_node::add_route_via_node(ip_subnet &prefix, network_interface *iface)
{
    // dao_needed = true;

    assert(mDN->dag_me != NULL);
    iface->add_route_to_node(prefix, this,
                             mDN->dag_me->prefix_number().addr);
}

rpl_node *network_interface::find_neighbour_by_grasp_sessionid(grasp_session_id gsi)
{
    node_map_iterator nmi = this->neighbours.begin();
    node_map_iterator nmi_end = this->neighbours.end();
    while(nmi != nmi_end) {
        if(nmi->second.queryId == gsi) {
            return &nmi->second;
        }
        nmi++;
    }

    return NULL;
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */






