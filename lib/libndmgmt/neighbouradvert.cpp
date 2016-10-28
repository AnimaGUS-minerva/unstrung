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

void rpl_node::reply_neighbour_advert(network_interface *iface,
                                      unsigned int success)
{
    unsigned char icmp_body[2048];

    na_construction buildit;
    buildit.buff     = icmp_body;
    buildit.buff_len = sizeof(icmp_body);
    start_neighbour_advert(buildit);
    buildit.nna->nd_na_flags_reserved |= ND_NA_FLAG_SOLICITED;
    buildit.nna->nd_na_target         = this->node_number();

    struct nd_opt_aro *noa = (struct nd_opt_aro *)buildit.nextopt;
    buildit.nextopt = (unsigned char *)(noa+1);

    unsigned char *aro64 = &this->nodeip.u.v6.sin6_addr.s6_addr[8];
    memcpy(noa->nd_aro_eui64, aro64, 8);

    noa->nd_aro_status   = success;
    noa->nd_aro_type     = ND_OPT_ARO;
    noa->nd_aro_len      = sizeof(*noa);

    unsigned int icmp_len = end_neighbour_advert(buildit);

    /* src set to NULL, because we received this via mcast, reply with our address on this iface */
    iface->send_raw_icmp(&this->nodeip.u.v6.sin6_addr, NULL, icmp_body, icmp_len);
}


void rpl_node::reply_mcast_neighbour_join(network_interface *iface,
                                          struct in6_addr from,
                                          struct in6_addr ip6_to,
                                          const  time_t now,
                                          struct nd_neighbor_solicit *ns,
                                          const int nd_len)
{
    debug->info("  %s is looking to join network\n", this->node_name());

    unsigned char *in_nextopt = (unsigned char *)(ns+1);
    unsigned char *optend     = ((unsigned char *)ns) + nd_len;
    struct nd_opt_aro *in_aro_opt = NULL;

    while(in_nextopt < (optend-sizeof(struct nd_opt_hdr))) {
        struct nd_opt_hdr *opt = (struct nd_opt_hdr *)in_nextopt;

        /* process the options */
        switch(opt->nd_opt_type) {
        case ND_OPT_SOURCE_LINKADDR:
            break;
        case ND_OPT_TARGET_LINKADDR:
        case ND_OPT_PREFIX_INFORMATION:
        case ND_OPT_REDIRECTED_HEADER:
        case ND_OPT_MTU:
        case ND_OPT_RTR_ADV_INTERVAL:
        case ND_OPT_HOME_AGENT_INFO:
            break;

        case ND_OPT_ARO:
            struct nd_opt_aro *noa1 = (struct nd_opt_aro *)opt;
            if(!in_aro_opt) in_aro_opt = noa1;
            debug->info("ARO with target: %02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                        noa1->nd_aro_eui64[0], noa1->nd_aro_eui64[1],
                        noa1->nd_aro_eui64[2], noa1->nd_aro_eui64[3],
                        noa1->nd_aro_eui64[4], noa1->nd_aro_eui64[5],
                        noa1->nd_aro_eui64[6], noa1->nd_aro_eui64[7]);
            break;
        }

        in_nextopt = (in_nextopt + (opt->nd_opt_len * 8));
    }

    if(!in_aro_opt) {
        return;
    }

    dag_network::globalStats[PS_NEIGHBOUR_JOIN_NEIGHBOUR_SOLICITATION]++;

    /* see if NCE says that node was already declined. */
    if(this->join_declined()) {
        dag_network::globalStats[PS_NEIGHBOUR_JOIN_QUERY_ALREADY_DECLINED]++;
        debug->log("sending declining Neighbour Advertisement on if: %s%s\n",
                   iface->get_if_name(),
                   iface->faked() ? "(faked)" : "");

        reply_neighbour_advert(iface, ND_NS_JOIN_DECLINED);
        return;
    }

    /* see if NCE says that the query is still in progress, in which case, do nothing */
    if(this->join_queryInProgress()) {
        dag_network::globalStats[PS_NEIGHBOUR_JOIN_QUERY_INPROGRESS]++;
        return;
    }

    if(this->join_accepted()) {
        dag_network::globalStats[PS_NEIGHBOUR_JOIN_QUERY_ALREADY_ACCEPTED]++;

        debug->log("sending accepting Neighbour Advertisement on if: %s%s\n",
                   iface->get_if_name(),
                   iface->faked() ? "(faked)" : "");

        reply_neighbour_advert(iface, 0);
        return;
    }

    dag_network::globalStats[PS_NEIGHBOUR_JOIN_QUERY_STARTED]++;
    /* better send a new query! */

    if(iface->join_query_client) {
        this->queryId = iface->join_query_client->start_query_for_aro(in_aro_opt->nd_aro_eui64);
    }
    /* reply to query will send return value */
    return;
}

void network_interface::process_grasp_reply(grasp_session_id gsi, bool success)
{
    rpl_node *rn = find_neighbour_by_grasp_sessionid(gsi);
    if(rn == NULL) return;

    debug->info("ending query from Registrar for %s", rn->node_name());

    rn->set_accepted(success);
    rn->reply_neighbour_advert(this, success ? 0 : ND_NS_JOIN_DECLINED);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */
