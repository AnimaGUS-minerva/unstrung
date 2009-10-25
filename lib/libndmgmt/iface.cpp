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
    const struct nd_opt_hdr *nd_options;
    char src_addrbuf[INET6_ADDRSTRLEN];
    char dst_addrbuf[INET6_ADDRSTRLEN];

    /* should collect this all into a "class packet", or "class transaction" */
    if(this->verbose_flag) {
	inet_ntop(AF_INET6, &ip6_src, src_addrbuf, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &ip6_dst, dst_addrbuf, INET6_ADDRSTRLEN);
    }

    struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)bytes;
    
    /* mark end of data received */
    const u_char *bytes_end = bytes + len;

    /* XXX should maybe check the checksum? */

    switch(icmp6->icmp6_type) {
    case ND_ROUTER_SOLICIT:
    {
	struct nd_router_solicit *nrs = (struct nd_router_solicit *)bytes;
	nd_options = (const struct nd_opt_hdr *)&nrs[1];
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
	nd_options = (const struct nd_opt_hdr *)&nra[1];
	if(VERBOSE(this)) {
	    fprintf(this->verbose_file, "Got router advertisement from %s (hoplimit: %u, flags=%s%s%s, lifetime=%ums, reachable=%u, retransmit=%u)\n  with %u bytes of options\n",
		    src_addrbuf,
		    nra->nd_ra_curhoplimit,
		    nra->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED ? "managed " : "",
		    nra->nd_ra_flags_reserved & ND_RA_FLAG_OTHER ? "other " : "",
		    nra->nd_ra_flags_reserved & ND_RA_FLAG_HOME_AGENT ? "home-agent " : "",
		    ntohs(nra->nd_ra_router_lifetime),
		    ntohl(nra->nd_ra_reachable),
		    ntohl(nra->nd_ra_retransmit),
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;

    case ND_NEIGHBOR_SOLICIT:
    {
	struct nd_neighbor_solicit *nns = (struct nd_neighbor_solicit *)bytes;
	nd_options = (const struct nd_opt_hdr *)&nns[1];
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
	nd_options = (const struct nd_opt_hdr *)&nna[1];
	if(VERBOSE(this)) {
	    char target_addrbuf[INET6_ADDRSTRLEN];
	    inet_ntop(AF_INET6, &nna->nd_na_target, target_addrbuf, INET6_ADDRSTRLEN);
	    fprintf(this->verbose_file, "Got neighbor advertisement from %s (flags=%s%s%s), advertising: %s, has %u bytes of options\n",
		    src_addrbuf,
		    nna->nd_na_flags_reserved & ND_NA_FLAG_ROUTER ? "router " : "",
		    nna->nd_na_flags_reserved & ND_NA_FLAG_SOLICITED ? "solicited " : "",
		    nna->nd_na_flags_reserved & ND_NA_FLAG_OVERRIDE  ? "override " : "",
		    
		    target_addrbuf,
		    bytes_end - (u_char *)nd_options);
	}
    }
    break;
    }

    /* now decode the option packets */
    while((const u_char *)nd_options < bytes_end && nd_options->nd_opt_type!=0) {
	/*
	 * option lengths are in units of 64-bits, the minimum size for
	 * an IPv6 extension
	 */
	unsigned int optlen = nd_options->nd_opt_len << 3;
	if(this->packet_too_short("option",
				  optlen,
				  sizeof(struct nd_opt_hdr))) return;

	if(this->packet_too_short("option",
				  (bytes_end - bytes),
				  optlen)) return;

	if(VERBOSE(this)) {
	    fprintf(this->verbose_file, "    option %u[%u]",
		    nd_options->nd_opt_type, optlen);
	}
	switch(nd_options->nd_opt_type) {
	case ND_OPT_SOURCE_LINKADDR:
	    if(VERBOSE(this)) fprintf(this->verbose_file, " source-linkaddr \n");
	    break;

	case ND_OPT_TARGET_LINKADDR:
	    if(VERBOSE(this)) fprintf(this->verbose_file, " target-linkaddr \n");
	    break;

	case ND_OPT_PREFIX_INFORMATION:
	{
	    struct nd_opt_prefix_info *nopi = (struct nd_opt_prefix_info *)nd_options;
	    if(this->packet_too_short("option",
				      optlen,
				      sizeof(struct nd_opt_prefix_info))) return;
	    
	    if(VERBOSE(this)) {
		char prefix_addrbuf[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &nopi->nd_opt_pi_prefix, prefix_addrbuf, INET6_ADDRSTRLEN);
		fprintf(this->verbose_file, " prefix %s/%u (valid=%us, preferred=%us) flags=%s%s%s\n",
			prefix_addrbuf, nopi->nd_opt_pi_prefix_len,
			nopi->nd_opt_pi_valid_time,
			nopi->nd_opt_pi_preferred_time,
			nopi->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_ONLINK ? "onlink " : "",
			nopi->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_AUTO   ? "auto " : "",
			nopi->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_RADDR  ? "raddr " : "");
	    }
	}
	break;

	case ND_OPT_REDIRECTED_HEADER:
	    if(VERBOSE(this)) fprintf(this->verbose_file, " redirected \n");
	    break;

	case ND_OPT_MTU:
	    if(VERBOSE(this)) fprintf(this->verbose_file, " mtu \n");
	    break;

	case ND_OPT_RTR_ADV_INTERVAL:
	    if(VERBOSE(this)) fprintf(this->verbose_file, " rtr-interval \n");
	    break;

	case ND_OPT_HOME_AGENT_INFO:
	    if(VERBOSE(this)) fprintf(this->verbose_file, " home-agent-info \n");
	    break;

	default:
	    /* nothing */
	    break;
	}
	
	const u_char *nd_bytes = (u_char *)nd_options;
	nd_bytes += optlen;
	nd_options = (const struct nd_opt_hdr *)nd_bytes;
    }
    
    
}

int network_interface::packet_too_short(const char *thing,
				       const int avail_len,
				       const int needed_len)
{
    if(avail_len < needed_len) {
	fprintf(this->verbose_file, "%s has invalid len: %u(<%u)\n",
		thing, avail_len, needed_len);
	
	this->error_cnt++;
	/* discard packet */
	return 1;
    }
    return 0;
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
