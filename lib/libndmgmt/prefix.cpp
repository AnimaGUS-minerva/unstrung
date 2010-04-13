/*
 * Copyright (C) 2010 Michael Richardson <mcr@sandelman.ca>
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
};

#include "dag.h"
#include "node.h"
#include "prefix.h"

prefix_node::prefix_node(rpl_node *announcer, ip_subnet sub) {
        name[0]='\0';
        mPrefix = sub;
        announced_from = announcer;
        valid = true;
}

void prefix_node::set_prefix(const struct in6_addr v6, const int prefixlen) {
    memset(&mPrefix, 0, sizeof(mPrefix));
    mPrefix.addr.u.v6.sin6_family = AF_INET6;
    mPrefix.addr.u.v6.sin6_flowinfo = 0;		/* unused */
    mPrefix.addr.u.v6.sin6_port = 0;
    memcpy((void *)&mPrefix.addr.u.v6.sin6_addr, (void *)&v6, 16);
    mPrefix.maskbits  = prefixlen;
    name[0]='\0';
    valid = true;
}

void prefix_node::set_prefix(ip_subnet prefix)
{
    mPrefix = prefix;
    name[0]='\0';
    valid = true;
}

const char *prefix_node::node_name() {
    if(valid) {
        if(name[0]) return name;

        subnettot(&mPrefix, 0, name, sizeof(name));
        return name;
    } else {
        return "<prefix-not-valid>";
    }
};

void prefix_node::configureip(void)
{
    this->verbose_log("  peer '%s' announces prefix: %s\n",
                      announced_from->node_name(), node_name());
}

void prefix_node::verbose_log(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    
    if(mDN->verboseprint()) {
        vfprintf(mDN->verbose_file, fmt, vargs);
    }
}



/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */


        



