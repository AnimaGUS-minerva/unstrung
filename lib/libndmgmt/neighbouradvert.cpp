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

/*
 * a NA will be sent as part of the join process to let other devices
 * know that it exists.
 * It will cause a DAR/DAC process to be initiated upwards.
 *
 */
class na_construction {
 public:
    unsigned char      *buff;
    unsigned int        buff_len;
    unsigned char      *nextopt;
    struct icmp6_hdr   *icmp6;
    struct nd_neighbor_advert *nna;
};

void rpl_node::start_neighbour_advert(na_construction &progress)
{
    memset(progress.buff, 0, progress.buff_len);

    progress.icmp6 = (struct icmp6_hdr *)progress.buff;
    progress.icmp6->icmp6_type = ND_NEIGHBOR_ADVERT;
    progress.icmp6->icmp6_code = 0;
    progress.icmp6->icmp6_cksum = 0;

    progress.nna = (struct nd_neighbor_advert *)progress.icmp6;
    progress.nextopt = (unsigned char *)(progress.nna+1);

    /*
     * ND_NA_FLAG_ROUTER    is off.
     */
    progress.nna->nd_na_flags_reserved = ND_NA_FLAG_OVERRIDE;
    return;
}

int rpl_node::end_neighbour_advert(na_construction &progress)
{
    int len = 0;

    /* calculate length */
    len = ((caddr_t)progress.nextopt - (caddr_t)progress.buff);

    len = (len + 7)&(~0x7);  /* round up to next multiple of 64-bits */
    return len;
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

    na_construction buildit;
    buildit.buff = buff;
    buildit.buff_len = buff_len;

    start_neighbour_advert(buildit);

    if(solicited) {
        buildit.nna->nd_na_flags_reserved |= ND_NA_FLAG_SOLICITED;
    }

    buildit.nna->nd_na_target         = iface->link_local();

    return end_neighbour_advert(buildit);
}

void rpl_node::reply_mcast_neighbour_advert(network_interface *iface,
                                            struct in6_addr from,
                                            struct in6_addr ip6_to,
                                            const  time_t now,
                                            struct nd_neighbor_solicit *ns,
                                            const int nd_len)
{
    debug->info("  NS looking for: %s\n", this->node_name());

    /* look up, on this interface, for the appropriate peer */
    update_nce_stamp();

    unsigned char icmp_body[2048];

    debug->log("sending Neighbour Advertisement on if: %s%s\n",
               iface->get_if_name(),
	       iface->faked() ? "(faked)" : "");
    memset(icmp_body, 0, sizeof(icmp_body));

    unsigned int icmp_len = build_basic_neighbour_advert(iface, true, icmp_body, sizeof(icmp_body));

    /* src set to NULL, because we received this via mcast, reply with our address on this iface */
    iface->send_raw_icmp(&from, NULL, icmp_body, icmp_len);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */
