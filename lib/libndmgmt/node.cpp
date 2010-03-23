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

        if(inet_pton(AF_INET6, ipv6, &nodeip) == 1) {
                valid=true;
        }
}

rpl_node::rpl_node(const struct in6_addr v6) {
        nodeip = v6;
        valid = true;
}

void rpl_node::makevalid(const struct in6_addr v6, const dag_network *dn)
{
    if(!valid) {
        nodeip = v6;
        mDN    = dn;
            
        if(1 /* mDN->iface->verbose_test()*/) {
            char src_addrbuf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &v6, src_addrbuf, INET6_ADDRSTRLEN);

            fprintf(stderr /* this->verbose_file*/, "  new RPL node: %s \n",
                    src_addrbuf);
        }
        valid  = true;
    }
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */


        



