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

#include <stdio.h>
#include <stdlib.h>

#include <netinet/ip6.h>

#include "iface.h"


network_interface::network_interface()
{
    nd_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
}

network_interface::network_interface(int fd)
{
    nd_socket = fd;
}

void network_interface::receive_packet(const u_char *bytes, const int len)
{
    return;
}

int network_interface::send_packet(const u_char *bytes, const int len)
{
    return len;
}



/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
