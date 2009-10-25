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

extern "C" {
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
}

#include "iface.h"


network_interface::network_interface()
{
    nd_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
}

network_interface::network_interface(int fd)
{
    nd_socket = fd;
}

void network_interface::receive_packet(struct in6_addr ip6_src,
				       struct in6_addr ip6_dst,
				       const u_char *bytes, const int len)
{
    u_char *nd_options;
    char src_addrbuf[INET6_ADDRSTRLEN];
    char dst_addrbuf[INET6_ADDRSTRLEN];

    /* should collect this all into a "class packet", or "class transaction" */
    if(this->verbose_flag) {
	inet_ntop(AF_INET6, &ip6_src, src_addrbuf, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &ip6_dst, dst_addrbuf, INET6_ADDRSTRLEN);
    }

    struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)bytes;
    const u_char *bytes_end = bytes + len;
    /* XXX should maybe check the checksum? */

    switch(icmp6->icmp6_type) {
    case ND_ROUTER_SOLICIT:
    {
	struct nd_router_solicit *nrs = (struct nd_router_solicit *)bytes;
	nd_options = (u_char *)&nrs[1];
	if(VERBOSE(this)) {
	    fprintf(this->verbose_file, "Got router solicitation from %s with %u bytes of options\n",
		    src_addrbuf,
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;

    case ND_ROUTER_ADVERT:
    {
	struct nd_router_advert *nra = (struct nd_router_advert *)bytes;
	nd_options = (u_char *)&nra[1];
	if(VERBOSE(this)) {
	    fprintf(this->verbose_file, "Got router advertisement (reachable=%u, retransmit=%u) from %s with %u bytes of options\n",
		    ntohl(nra->nd_ra_reachable),
		    ntohl(nra->nd_ra_retransmit),
		    src_addrbuf,
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;

    case ND_NEIGHBOR_SOLICIT:
    {
	struct nd_neighbor_solicit *nns = (struct nd_neighbor_solicit *)bytes;
	nd_options = (u_char *)&nns[1];
	if(VERBOSE(this)) {
	    char target_addrbuf[INET6_ADDRSTRLEN];
	    inet_ntop(AF_INET6, &nns->nd_ns_target, target_addrbuf, INET6_ADDRSTRLEN);
	    fprintf(this->verbose_file, "Got neighbour solicitation from %s, looking for %s, has %u bytes of options\n",
		    src_addrbuf,
		    target_addrbuf,
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;
	
    case ND_NEIGHBOR_ADVERT:
    {
	struct nd_neighbor_advert *nna = (struct nd_neighbor_advert *)bytes;
	nd_options = (u_char *)&nna[1];
	if(VERBOSE(this)) {
	    char target_addrbuf[INET6_ADDRSTRLEN];
	    inet_ntop(AF_INET6, &nna->nd_na_target, target_addrbuf, INET6_ADDRSTRLEN);
	    fprintf(this->verbose_file, "Got neighbor advertisement from %s, advertising: %s, has %u bytes of options\n",
		    src_addrbuf,
		    target_addrbuf,
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;
    }

    /* now decode neighbour discovery packets */
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
