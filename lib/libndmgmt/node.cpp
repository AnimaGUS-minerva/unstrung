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


