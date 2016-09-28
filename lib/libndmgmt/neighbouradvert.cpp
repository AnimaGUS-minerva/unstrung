/*
 * Copyright (C) 2016 Michael Richardson <mcr@sandelman.ca>
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
#include <string.h>

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <time.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include "oswlibs.h"
#include "rpl.h"
#include "neighbour.h"
}

#include "iface.h"
#include "devid.h"

void network_interface::receive_neighbour_advert(struct in6_addr from,
                                                 struct in6_addr ip6_to,
                                                 const  time_t now,
                                                 const u_char *dat, const int nd_len)
{
    unsigned int dat_len = nd_len;
    debug->info("  processing ND(%u)\n",nd_len);
}

void network_interface::reply_neighbour_advert(struct in6_addr from,
                                               struct in6_addr ip6_to,
                                               const  time_t now,
                                               struct nd_neighbor_solicit *ns, const int nd_len)
{
    /* look up the node by from address, if the from is not :: */
    if(memcmp(&in6addr_any, &from, 16)==0) {
        dag_network::globalStats[PS_NEIGHBOUR_UNICAST_SOURCE_UNSPECIFIED]++;
        return;
    }

    /* look for an SLLAO option in the message */
    rpl_node &source = this->neighbours[from];

    source.markvalid(this->get_if_index(), from, this->debug);
    source.update_nce_stamp();

    /* respond with NA for us, which confirms reachability */
    debug->info("  sending NA to: %s\n", source.node_name());

}

void network_interface::reply_mcast_neighbour_advert(rpl_node &neighbour,
                                                     struct in6_addr from,
                                                     struct in6_addr ip6_to,
                                                     const  time_t now,
                                                     struct nd_neighbor_solicit *ns,
                                                     const int nd_len)
{
    debug->info("  NS looking for: %s\n", neighbour.node_name());

    /* look up, on this interface, for the appropriate peer */


}

/*
 * a NA will be sent as part of the join process to let other devices
 * know that it exists.
 * It will cause a DAR/DAC process to be initiated upwards.
 *
 */
int rpl_node::build_basic_neighbour_advert(network_interface *iface,
                                           bool solicited,
                                           unsigned char *buff,
                                           unsigned int buff_len)
{
    struct sockaddr_in6 addr;
    struct in6_addr *dest = NULL;
    struct icmp6_hdr  *icmp6;
    struct nd_neighbor_advert *nna;
    unsigned char *nextopt;
    int optlen;
    int len = 0;

    memset(buff, 0, buff_len);

    icmp6 = (struct icmp6_hdr *)buff;
    icmp6->icmp6_type = ND_NEIGHBOR_ADVERT;
    icmp6->icmp6_code = 0;
    icmp6->icmp6_cksum = 0;

    nna = (struct nd_neighbor_advert *)icmp6;
    nextopt = (unsigned char *)(nna+1);

    /*
     * ND_NA_FLAG_ROUTER    is off.
     */
    nna->nd_na_flags_reserved = ND_NA_FLAG_OVERRIDE;
    if(solicited) {
        nna->nd_na_flags_reserved |= ND_NA_FLAG_SOLICITED;
    }

    nna->nd_na_target         = iface->link_local();

    /* calculate length */
    len = ((caddr_t)nextopt - (caddr_t)buff);

    len = (len + 7)&(~0x7);  /* round up to next multiple of 64-bits */
    return len;
}

#if 0
void network_interface::send_na(device_identity &di)
{
    unsigned char icmp_body[2048];

    debug->log("sending Neighbour Advertisement on if: %s%s\n",
               this->if_name,
	       this->faked() ? "(faked)" : "");
    memset(icmp_body, 0, sizeof(icmp_body));

    unsigned int icmp_len = di.build_neighbour_advert(this, icmp_body, sizeof(icmp_body));

    struct in6_addr all_hosts_inaddr;
    memcpy(all_hosts_inaddr.s6_addr, all_hosts_addr, 16);
    send_raw_icmp(&all_hosts_inaddr, NULL, icmp_body, icmp_len);
}
#endif

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */
