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
};

#include "node.h"

rpl_node::rpl_node(const char *ipv6) {
        valid = false;
        name[0]='\0';

        if(inet_pton(AF_INET6, ipv6, &nodeip) == 1) {
                valid=true;
        }
}

rpl_node::rpl_node(const struct in6_addr v6) {
        nodeip = v6;
        valid = true;
        name[0]='\0';
}

const char *rpl_node::node_name() {
    if(valid) {
        if(name[0]) return name;

        inet_ntop(AF_INET6, &nodeip, name, INET6_ADDRSTRLEN);
        return name;
    } else {
        return "<node-not-valid>";
    }
};

void rpl_node::makevalid(const struct in6_addr v6,
                         const dag_network *dn,
                         rpl_debug *deb)
{
    if(!valid) {
        nodeip = v6;
        mDN    = dn;
        valid  = true;
        this->debug  = deb;
        
        if(debug->verbose_test()) {
            fprintf(debug->file, "  new RPL node: %s \n",
                    node_name());
        }
    }
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */


        



