/*
 * Copyright (C) 2009-2013 Michael Richardson <mcr@sandelman.ca>
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
#include <limits.h>

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
    #include <fnmatch.h>
#include "oswlibs.h"
#include "rpl.h"

}

#include "iface.h"
#include "dag.h"
#include "dio.h"
#include "dao.h"

class dag_network *dag_network::all_dag = NULL;
u_int32_t dag_network::globalStats[PS_MAX];
u_int32_t dag_network::globalOldStats[PS_MAX];

unsigned char           dag_network::optbuff[256];
unsigned int            dag_network::optlen;


void dag_network::format_dagid(char *dagidstr,
                               unsigned int dagidstr_len,
                               instanceID_t   rpl_instance,
			       const u_int8_t rpl_dagid[DAGID_LEN])
{
    char buf[64];

    inet_ntop(AF_INET6, rpl_dagid, buf, 64);
    snprintf(dagidstr, dagidstr_len, "%u/[%s]", rpl_instance, buf);
}

void dag_network::init_stats(void)
{
}

void dag_network::init_dag_name(void)
{
    memset(mDagName, 0, sizeof(mDagName));
    format_dagid(mDagName, sizeof(mDagName), mInstanceid, mDagid);
}

void dag_network::init_dag(void)
{
    mLastSeq = 0;
    mMyRank   = UINT_MAX;
    mBestRank = UINT_MAX;
    mVersion  = 1;
    debug     = NULL;
    mSendDioEvent = NULL;
    mSendDaoEvent = NULL;
    myDeviceIdentity = NULL;
    memset(mStats,     0, sizeof(mStats));
    memset(old_mStats, 0, sizeof(old_mStats));
    mMode = RPL_DIO_STORING_MULTICAST;
    mDAOSequence = 1;
    mDTSN = INVALID_SEQUENCE;
    init_dag_name();
    this->add_to_list();
    dag_me = NULL;
    mPrefixSet = false;
    mIfWildcard_max = 0;
}

dag_network::dag_network(instanceID_t num, dagid_t n_dagid, rpl_debug *deb)
{
    set_instanceid(num);
    set_dagid(n_dagid);
    init_dag();
    set_debug(deb);
}

dag_network::dag_network(instanceID_t num, struct in6_addr *addr, rpl_debug *deb)
{
    set_instanceid(num);
    set_dagid(addr->s6_addr);
    init_dag();
    set_debug(deb);
}


dag_network::dag_network(instanceID_t num, const char *s_dagid, rpl_debug *deb)
{
    set_dagid(s_dagid);
    set_instanceid(num);
    init_dag();
    set_debug(deb);
}

dag_network::dag_network(instanceID_t num, rpl_debug *deb)
{
    set_instanceid(num);
    init_dag();
    set_debug(deb);
}



dag_network::~dag_network()
{
        this->remove_from_list();
}

void dag_network::set_prefix(const struct in6_addr v6, const int prefixlen)
{
    memset(&mPrefix, 0, sizeof(mPrefix));
    mPrefix.addr.u.v6.sin6_family = AF_INET6;
    mPrefix.addr.u.v6.sin6_flowinfo = 0;		/* unused */
    mPrefix.addr.u.v6.sin6_port = 0;
    memcpy((void *)&mPrefix.addr.u.v6.sin6_addr, (void *)&v6, 16);
    mPrefix.maskbits  = prefixlen;
    mPrefixName[0]='\0';
    mPrefixSet = true;
}

void dag_network::set_grounded(bool grounded)
{
    if(grounded) {
        this->mGrounded = true;

        /* now add this prefix as a blackhole route on lo */
        if(loopback_interface && mPrefixSet)
            loopback_interface->add_null_route_to_prefix(mPrefix);
    } else {
        this->mGrounded = false;

        /* now add this prefix as a blackhole route on lo */
        /* loopback_interface->delete_null_route_to_prefix(mPrefix); */
    }
}

void dag_network::set_prefix(const ip_subnet prefix)
{
    this->set_prefix(prefix.addr.u.v6.sin6_addr, prefix.maskbits);
}

const char *dag_network::prefix_name() {
    if(mPrefixName[0] == '\0') {
        subnettot(&mPrefix, 0, mPrefixName, sizeof(mPrefixName));
    }
    return mPrefixName;
};


void dag_network::remove_from_list(void)
{
        class dag_network **p_dn = &dag_network::all_dag;

        while(*p_dn != NULL && *p_dn != this) {
                class dag_network *dn = *p_dn;
                p_dn = &dn->next;
        }

        /* did we find it? */
        if(*p_dn == this) {
                /* yes */
                *p_dn = this->next;
        }
}


class dag_network *dag_network::find_or_make_by_instanceid(instanceID_t num,
                                                           dagid_t n_dagid,
                                                           rpl_debug *debug,
                                                           bool watching)
{
    class dag_network *dn = find_by_instanceid(num, n_dagid);

    if(dn==NULL) {
        dn = new dag_network(num, n_dagid, debug);
        if(watching) {
            globalStats[PS_DAG_CREATED_FOR_WATCHING]++;
        }
        dn->set_inactive();             /* in active by default */
    }
    return dn;
}

class dag_network *dag_network::find_or_make_by_instanceid(instanceID_t num,
                                                           struct in6_addr addr,
                                                           rpl_debug *debug,
                                                           bool watching)
{
    dagid_t n_dagid;

    memcpy(n_dagid, addr.s6_addr, DAGID_LEN);
    return find_or_make_by_instanceid(num,n_dagid,debug,watching);
}

class dag_network *dag_network::find_or_make_by_string(instanceID_t num,
                                                       const char *dagid,
						       rpl_debug *debug,
						       bool watching)
{
    int len = strlen(dagid);
    dagid_t d;

    memset(d, 0, sizeof(dagid_t));
    if(len > sizeof(dagid_t)) {
	len = sizeof(dagid_t);
    }
    memcpy(d, dagid, len);
    return find_or_make_by_instanceid(num, d, debug, watching);
}

class dag_network *dag_network::find_by_instanceid(instanceID_t num, dagid_t n_dagid)
{
    class dag_network *dn = dag_network::all_dag;

    while(dn != NULL) {
        if(dn->mInstanceid == num) break;
        dn = dn->next;
    }
    return dn;
}

class dag_network *dag_network::find_by_instanceid(instanceID_t num)
{
    class dag_network *dn = dag_network::all_dag;

    while(dn != NULL) {
        if(dn->mInstanceid == num) break;
        dn = dn->next;
    }
    return dn;
}

bool dag_network::matchesIfWildcard(const char *ifname)
{
    for(int i=0; i<mIfWildcard_max && i<DAG_IFWILDCARD_MAX; i++) {
        bool matched = fnmatch(mIfWildcard[i], ifname, FNM_CASEFOLD)==0;
        debug->debug(RPL_DEBUG_NETLINK, "matching if:%s against %s: result: %s\n",
                     ifname, mIfWildcard[i], matched ? "matched" : "failed");
        if(matched) return true;
    }
    return false;
}

bool dag_network::matchesIfPrefix(const struct in6_addr v6)
{
    ip_address n6;
    n6.u.v6.sin6_family = AF_INET6;
    n6.u.v6.sin6_addr = v6;
    return matchesIfPrefix(n6);
}

bool dag_network::matchesIfPrefix(const ip_address v6)
{
    char b1[ADDRTOT_BUF], b2[SUBNETTOT_BUF];
    addrtot(&v6, 0, b1, sizeof(b1));
    for(int i=0; i<mIfFilter_max && i<DAG_IFWILDCARD_MAX; i++) {
        bool matched = addrinsubnet(&v6, &mIfFilter[i]);
        subnettot(&mIfFilter[i], 0, b2, sizeof(b2));
        debug->debug(RPL_DEBUG_NETLINK, "matching addr:%s against %s: result: %s\n",
                     b1, b2, matched ? "matched" : "failed");
        if(matched) return true;
    }
    return false;
}

bool dag_network::notify_new_interface(network_interface *one)
{
    bool announced = false;

    for(class dag_network *dn = dag_network::all_dag;
        dn != NULL;
        dn = dn->next) {
        if(dn->matchesIfWildcard(one->get_if_name()) &&
           dn->matchesIfPrefix(one->if_addr)) {

            announced = true;
            prefix_node *n = dn->add_address(one->node->node_address());
            n->set_prefix(one->if_addr, 128);
            n->set_debug(dn->debug);
            dn->debug->info("added %s to DAG %s\n",
                          one->node->node_name(),
                          dn->get_dagName());
            dn->dao_needed = true;
            dn->maybe_send_dao();
        }
    }
    return announced;
}

static void print_mStats(FILE *out, const char *prefix,
                    u_int32_t mStats[PS_MAX],
                    u_int32_t old_mStats[PS_MAX])
{
    int i;

    for(i=0; i<PS_MAX; i++) {
	fprintf(out, "%s %04u (+%04d) %s\n", prefix,
		mStats[i],
		mStats[i] - old_mStats[i],
		dag_network_packet_stat_names[i]);
	old_mStats[i] = mStats[i];
    }
}

void dag_network::print_stats(FILE *out, const char *prefix)
{
    print_mStats(out, prefix, mStats, old_mStats);
}

void dag_network::print_all_dagstats(FILE *out, const char *prefix)
{
    char prefixbuf2[64];

    print_mStats(out, "global", globalStats, globalOldStats);
    for(class dag_network *dn = dag_network::all_dag;
	dn != NULL;
	dn = dn->next) {
	snprintf(prefixbuf2, sizeof(prefixbuf2), "%s(%s) ",
		 prefix, dn->get_dagName());
	dn->print_stats(out, prefixbuf2);
    }
}

void dag_network::send_all_dag(network_interface *ni)
{
    class dag_network *dn = dag_network::all_dag;
    while(dn != NULL) {
        ni->send_dis(dn);
        dn = dn->next;
    }
}


void dag_network::discard_dio(enum packet_stats dr)
{
    mStats[dr]++;

    if(VERBOSE(this))
        fprintf(this->verbose_file, "  DIO discarded %s (++%u)\n",
                dag_network_packet_stat_names[dr], mStats[dr]);
}

/*
 * sequence numbers are kept as 32-bit integers, but when
 * transmitted, only the lower 8 bits matters, so they wrap.
 */
bool dag_network::seq_too_old(unsigned int seq)
{
    unsigned int low8bit = mLastSeq & 0xff;
    int diff = (seq - low8bit);

    /* if the seq > low8bit, everything is good */
    if(diff > 0 && diff < 128) {
        return false;
    }

    /* if the -128 < diff < 0, then we have a wrap backwards */
    if(diff < 0 && diff > -128) {
        return true;
    }

    /*
     * if there is too much space between them, then maybe it is an
     * old sequence number being replayed at us.
     */
    if((low8bit < 128) &&
       (seq > 128) &&
       (diff > 128)) {
        return true;
    }

    /*
     * do not update the sequence number here, we need
     * to check the security first!
     */
    return false;
}

void dag_network::seq_update(unsigned int seq)
{
    unsigned int low8bit = mLastSeq & 0xff;

    if(seq > low8bit) {
        mLastSeq += (seq - low8bit);
    } else if((seq+256) > low8bit) {
        mLastSeq += (seq+256 - low8bit);
    }
}

bool dag_network::check_security(const struct nd_rpl_dio *dio, int dio_len)
{
    /* XXXX */
    return true;
}

bool dag_network::check_security(const struct nd_rpl_dao *dao, int dao_len)
{
    /* XXXX */
    return true;
}

/* here we mark that a DAO is needed soon */
void dag_network::maybe_send_dao(void)
{
    if(dao_needed && dag_bestparent != NULL) {
        if(!root_node()) {
            schedule_dao();
        }
        dao_needed = false;
    }
}

void dag_network::add_childnode(rpl_node          *announcing_peer,
                                network_interface *iface,
                                ip_subnet prefix)
{
    prefix_node &pre = this->dag_children[prefix];
    char b1[256];
    subnettot(&prefix, 0, b1, 256);
    pre.set_debug(this->debug);

    if(!pre.is_installed()) {
        dao_needed = true;
        pre.set_prefix(prefix);
        pre.set_announcer(announcing_peer);

        announcing_peer->add_route_via_node(prefix, iface);
        set_dao_needed();
        pre.set_installed(true);
    }
#if 0
    debug->verbose("added child node %s/%s from %s\n",
                   b1, pre.node_name(),
                   announcing_peer->node_name());
#endif
}

prefix_node *dag_network::add_address(const ip_address addr)
{
    ip_subnet vs;

    initsubnet(&addr, 128, 0, &vs);
    return add_address(vs);
}

prefix_node *dag_network::add_address(const ip_subnet prefix)
{
    return &this->dag_announced[prefix];
}

void dag_network::cfg_new_node(prefix_node *me,
                               network_interface *iface,
                               ip_subnet prefix)
{
    if(!me->is_installed()) {
        me->set_debug(this->debug);

        dao_needed = true;
        me->set_prefix(prefix);

        if(iface != NULL) {
            me->configureip(iface, this);
        }

        if(dag_me == NULL) {
            dag_me = me;
        }
    }
}

void dag_network::set_acp_identity(device_identity *di) {
    myDeviceIdentity = di;
    prefix_node *me = add_address(di->sn);
    cfg_new_node(me, NULL, di->sn);
}

void dag_network::add_prefix(rpl_node advertising_peer,
                             network_interface *iface,
                             ip_subnet prefix)
{
    prefix_node &pre = this->dag_prefixes[prefix];
    pre.set_debug(this->debug);

    if(!pre.is_installed()) {
        this->set_prefix(prefix);
        pre.set_prefix(prefix);
        pre.set_installed(true);
        pre.set_announcer(&advertising_peer);
        dao_needed = true;

        maybe_schedule_dio();
    }

    /* next, see if we should configure an address in this prefix */
    if(!mIgnorePio) {
        prefix_node *preMe = add_address(prefix);
        preMe->set_announcer(&advertising_peer);
        cfg_new_node(preMe, iface, prefix);
    }
}

/*
 * This pretends that we got a DIO on the relevant interface, and
 * does all of the appropriate configuration.
 * It should be used on ROOT nodes.
 *
 * There is another case where a DAO is going to be emitted out
 * an interface different than where the DIO received was, and that
 * case is not yet dealt with here.
 *
 * This is also used by senddao to initialize self.
 */
void dag_network::addselfprefix(network_interface *iface)
{
    rpl_node *me = find_or_make_member(iface->if_addr);
    me->makevalid(iface->if_addr, this, this->debug);
    me->markself(iface->get_if_index());

    prefix_node &pre = this->dag_prefixes[mPrefix];
    pre.set_debug(this->debug);

    //this->debug->warn("self installed: %u\n", pre.is_installed());
    if(!pre.is_installed()) {
        dao_needed = true;
        pre.set_prefix(mPrefix);
        pre.set_announcer(me);
        pre.configureip(iface, this);
        if(dag_me == NULL) {
            dag_me = &pre;
        }
    }
}

static int addselfprefix_each(network_interface *iface, void *arg)
{
    dag_network *that = (dag_network *)arg;
    //that->debug->warn("selfprefix: %s\n", iface->get_if_name());
    that->addselfprefix(iface);
    return 1;
}

void dag_network::add_all_interfaces(void)
{
    network_interface::foreach_if(addselfprefix_each, this);
}


void dag_network::potentially_lower_rank(rpl_node &peer,
                                         network_interface *iface,
                                         const struct nd_rpl_dio *dio,
                                         int dio_len)
{
    unsigned int rank = ntohs(dio->rpl_dagrank);

    debug->verbose("  does peer '%s' have better rank? (%u < %u)\n",
                   peer.node_name(), rank, mBestRank);

    this->mStats[PS_LOWER_RANK_CONSIDERED]++;

    if(rank > mBestRank) {
        this->mStats[PS_LOWER_RANK_REJECTED]++;
        return;
    }

    debug->verbose("  Yes, '%s' has best rank %u\n",
                   peer.node_name(), rank);

    if(dag_bestparent == &peer || dag_parent == &peer) {
	debug->verbose("  But it is the same parent as before: ignored\n");
        this->mStats[PS_SAME_PARENT_IGNORED]++;
	return;
    }
    /* XXX
     * this is actually quite a big deal (SEE rfc6550), setting my RANK.
     * just fake it for now by adding 1.
     */
    if(mDTSN != INVALID_SEQUENCE && mDTSN >= dio->rpl_dtsn) {
	debug->verbose("  Same sequence number, ignore\n");
        this->mStats[PS_SAME_SEQUENCE_IGNORED]++;
	return;
    }

    mDTSN     = dio->rpl_dtsn;
    mBestRank     = rank;

    /* XXX
     * this is actually quite a big deal (SEE rfc6550), setting my RANK.
     * just fake it for now by adding 1.
     */
    mMyRank       = rank + 1;   // XXX
    mGrounded     = RPL_DIO_GROUNDED(dio->rpl_mopprf);
    mInstanceid   = dio->rpl_instanceid;
    mVersion      = dio->rpl_version;
    mMode         = RPL_DIO_MOP(dio->rpl_mopprf);

    set_dagid(*(dagid_t *)dio->rpl_dagid);

    dag_bestparentif = iface;
    dag_bestparent   = &peer;

    /* now see if we have already an address on this new network */
    /*
     * to do this, we have to crack open the DIO.  UP to this point
     * we haven't taken the DIO apart, so do, keeping stuff on the stack.
     */
    rpl_dio decoded_dio(peer, this, dio, dio_len);

    unsigned int optcount = 0;

    struct rpl_dio_destprefix *dp;
    while((dp = decoded_dio.destprefix()) != NULL) {
        unsigned char v6bytes[16];
        int prefixbytes = ((dp->rpl_dio_prefixlen+7) / 8);
        ip_subnet prefix;
        prefix.maskbits = dp->rpl_dio_prefixlen;
        memset(v6bytes, 0, 16);
        memcpy(v6bytes, dp->rpl_dio_prefix, prefixbytes);
        initaddr(v6bytes, 16, AF_INET6, &prefix.addr);

        optcount++;
        add_prefix(peer, iface, prefix);
    }
    debug->verbose("  processed %u pio options\n", optcount);

    /* now schedule sending out packets */
    if(dag_parent) {
        /* can only send DIOs once we are sure about our parent! */
        maybe_schedule_dio();
    }
    maybe_send_dao();
}

/*
 * send out outgoing DAO: this also causes us to potentially commit to a parent.
 * (XXX: hard coded to wait for DAOACK)
 */
void dag_network::send_dao(void)
{
    int cnt = 0;
    prefix_map_iterator pi = dag_children.begin();
    while(pi != dag_children.end()) {
	prefix_node &pm = pi->second;

        debug->verbose("SENDING[%u] dao about %s for %s to: %s on if=%s\n",
                       cnt, pm.node_name(),
                       mDagName, dag_bestparent->node_name(),
                       dag_bestparentif ? dag_bestparentif->get_if_name():"unknown");
        cnt++;
        pi++;
    }
    pi = dag_announced.begin();
    while(pi != dag_announced.end()) {
	prefix_node &pm = pi->second;

        debug->verbose("SENDING[%u] dao about %s for %s to: %s on if=%s\n",
                       cnt, pm.node_name(),
                       mDagName,
                       dag_bestparent   ? dag_bestparent->node_name():"unknown",
                       dag_bestparentif ? dag_bestparentif->get_if_name():"unknown");
        cnt++;
        pi++;
    }

    if(dag_bestparent == NULL || dag_bestparentif ==NULL) return;

    /* need to tell our parent about how to reach us */
    dag_bestparentif->send_dao(*dag_bestparent, *this);

    if(0) {
        commit_parent();
    }
}

/*
 * this routine determines the need for sending out
 * a DIO sooner than we would otherwise.
 */
void dag_network::maybe_schedule_dio(void)
{
    /* the list of things that could change needs to be tracked */
    if(dag_lastparent != dag_parent) {
        schedule_dio(1);             /* send it almost immediately */
        dag_lastparent = dag_parent;
    }
}

void dag_network::schedule_dio(void)
{
    schedule_dio(mInterval_msec);
}

void dag_network::schedule_dio(unsigned int msec)
{
    struct timeval now;
    gettimeofday(&now, NULL);

    /* do nothing if there is no valid DAG */
    if(dag_rank_infinite()) return;

    debug->verbose("Scheduling dio in %u ms\n", msec+1);

    if(!mSendDioEvent) {
	mSendDioEvent = new rpl_event(0, mInterval_msec,
                                      rpl_event::rpl_send_dio,
				      mDagName, this->debug);
    }
    mSendDioEvent->event_type = rpl_event::rpl_send_dio;
    mSendDioEvent->set_alarm(now, 0, msec+1);

    mSendDioEvent->mDag = this;
    mSendDioEvent->requeue(network_interface::things_to_do);
    //this->dio_event = re;        // needs to be smart-pointer

}

void dag_network::clear_event(rpl_event *thisone)
{
    if(thisone == NULL ||
       mSendDaoEvent == thisone) {
        /* do not use delete... need reference counts XXX */
        mSendDaoEvent = NULL;
    }
}

/*
 * this routine sets up to send a DAO on a regular basis,
 * the first one goes out much sooner than normal (almost immediately).
 *
 */
void dag_network::schedule_dao(void)
{
    if(network_interface::terminating_soon) return;
    debug->verbose("Scheduling dao in %u ms\n", 2);

    if(!mSendDaoEvent) {
	mSendDaoEvent = new rpl_event(0, mInterval_msec,
                                      rpl_event::rpl_send_dao,
				      mDagName, this->debug);
        assert(mSendDaoEvent->inQueue == false);
    }

    mSendDaoEvent->event_type = rpl_event::rpl_send_dao;

    struct timeval now;
    gettimeofday(&now, NULL);
    mSendDaoEvent->set_alarm(now, 0, 2);

    mSendDaoEvent->mDag = this;
    mSendDaoEvent->requeue(network_interface::things_to_do);
}

rpl_node *dag_network::update_route(network_interface *iface,
				    ip_subnet &prefix, const time_t now)
{
    struct in6_addr *from = &prefix.addr.u.v6.sin6_addr;

    rpl_node &peer = this->dag_members[*from];
    peer.makevalid(*from, this, this->debug);

    peer.set_last_seen(now);
    return &peer;
}


rpl_node *dag_network::update_parent(network_interface *iface,
                                     struct in6_addr from,
                                     struct in6_addr ip6_to,
                                     const time_t now)
{
    /* no difference for parent or child for now */
    return update_node(iface, from, ip6_to, now);
}

rpl_node *dag_network::update_child(network_interface *iface,
				    struct in6_addr from,
                                    struct in6_addr ip6_to,
                                    const time_t now)
{
    /* no difference for parent or child for now */
    return update_node(iface, from, ip6_to, now);
}

/* called when */
rpl_node *dag_network::update_node(network_interface *iface,
                                   struct in6_addr from,
                                   struct in6_addr ip6_to,
                                   const time_t now)
{
    rpl_node &peer = this->dag_members[from];

    /* set debug early */
    peer.debug = this->debug;

    if(peer.isself() ||
       iface->if_ifaddr(from)) {
        peer.markself(iface->get_if_index());

        this->mStats[PS_SELF_PACKET_RECEIVED]++;
        iface->log_received_packet(from, ip6_to);
        debug->debug(RPL_DEBUG_NETINPUT, "  received self packet (%u/%u)\n",
		       this->mStats[PS_SELF_PACKET_RECEIVED],
		       this->mStats[PS_PACKET_RECEIVED]);
        return NULL;
    }

    peer.makevalid(from, this, this->debug);
    peer.set_last_seen(now);

    return &peer;
}

void dag_network::dump_dio(rpl_debug *debug, const struct nd_rpl_dio *dio)
{
    char dagname[16*6];

    format_dagid(dagname, sizeof(dagname), dio->rpl_instanceid, dio->rpl_dagid);
    debug->info_more(" [%s: seq:%u,rank:%u,version:%u,%s%s]\n",
                     dagname,
                     dio->rpl_dtsn, ntohs(dio->rpl_dagrank),
                     dio->rpl_version,
                     (RPL_DIO_GROUNDED(dio->rpl_mopprf) ? "grounded," : ""),
                     dag_network::mop_decode(dag_network::mop_extract(dio)));
}

/*
 * Process an incoming DODAG Information Object (DIO)
 * the DIO is the downward announcement.
 *
 */
void dag_network::receive_dis(network_interface *iface,
                              struct in6_addr from,
                              struct in6_addr ip6_to,
                              const time_t    now,
                              const struct nd_rpl_dis *dis, int dis_len)
{
    char b1[ADDRTOT_BUF], b2[ADDRTOT_BUF];
    inet_ntop(AF_INET6, &from,   b1, sizeof(b1));
    inet_ntop(AF_INET6, &ip6_to, b2, sizeof(b2));

    /* increment stat of number of packets processed */
    this->mStats[PS_PACKET_RECEIVED]++;
    this->mStats[PS_DIS_PACKET_RECEIVED]++;

    debug->warn("received DIS from %s <timer reset>\n", b1);

    /* now reset timers, and send out DIO */
    schedule_dio(1);
}

/*
 * Process an incoming DODAG Information Object (DIO)
 * the DIO is the downward announcement.
 *
 */
void dag_network::receive_dio(network_interface *iface,
                              struct in6_addr from,
                              struct in6_addr ip6_to,
                              const time_t    now,
                              const struct nd_rpl_dio *dio, int dio_len)
{
    /* it has already been checked to be at least sizeof(*dio) */
    int dio_payload_len = dio_len - sizeof(*dio);

    /* increment stat of number of packets processed */
    this->mStats[PS_PACKET_RECEIVED]++;
    this->mStats[PS_DIO_PACKET_RECEIVED]++;

    /* check the sequence number.  But, if it's the first packet,
       accept it anyway. This still has a problem if we "sleep" for too
       long, but in that case, perhaps we should have issued a DIS.
       Maybe issue a DIS whenever this condition gets tripped?
    */
    if(this->seq_too_old(dio->rpl_dtsn)
       && dio->rpl_dtsn != 0
       && this->mStats[PS_DIO_PACKET_RECEIVED]!=1) {

        this->discard_dio(PS_SEQ_OLD);
        return;
    }

    /* validate this packet */
    bool secure = this->check_security(dio, dio_len);

    rpl_node *peer;

    /* find the node entry from this source IP, and update seen time */
    /* this will create the node if it does not already exist! */
    if((peer = this->update_parent(iface, from, ip6_to, now)) == NULL) {
        return;
    }

    iface->log_received_packet(from, ip6_to);
    debug->info(" processing dio(%u) ",dio_len);
    dump_dio(debug, dio);


    this->seq_update(dio->rpl_dtsn);

    if(mActive == false) {
	this->mStats[PS_PACKETS_WATCHED]++;
	return;
    }

    /* must be an active DAG, so see what we should do */
    this->potentially_lower_rank(*peer, iface, dio, dio_len);

    /* increment stat of number of packets processed */
    this->mStats[PS_PACKET_PROCESSED]++;
}

rpl_node *dag_network::find_or_make_member(const struct in6_addr memberaddr)
{
    return &this->dag_members[memberaddr];
}

rpl_node *dag_network::get_member(const struct in6_addr memberaddr)
{
    node_map_iterator ni = dag_members.find(memberaddr);

    if(ni == dag_members.end()) return NULL;
    else return &ni->second;
}

void dag_network::set_dagid(const char *dagstr)
{
    if(dagstr[0]=='0' && dagstr[1]=='x') {
        const char *digits;
        int i;
        digits = dagstr+2;
        for(i=0; i<16 && *digits!='\0'; i++) {
            unsigned int value;
            if(sscanf(digits, "%2x",&value)==0) break;
            mDagid[i]=value;

            /* advance two characters, carefully */
            digits++;
            if(digits[0]) digits++;
        }
        init_dag_name();
    } else {
        struct in6_addr dagnum;
        dagid_t n;

        if(inet_pton(AF_INET6, dagstr, &dagnum)==1) {
            memcpy(&n, dagnum.s6_addr, 16);
        }
        set_dagid(n);
    }
}

void dag_network::set_dagid(dagid_t dag)
{
    memcpy(this->mDagid, dag, DAGID_LEN);
    init_dag_name();
}




/*
 * Process an incoming DAG Advertisement Object.
 * the DAO is the upward announcement.
 *
 */
void dag_network::receive_dao(network_interface *iface,
                              struct in6_addr from,
                              struct in6_addr ip6_to,
                              const time_t    now,
                              const struct nd_rpl_dao *dao,
                              unsigned char *data, int dao_len)
{
    /* it has already been checked to be at least sizeof(*dio) */
    //int dao_payload_len = dao_len - sizeof(*dao);

    /* increment stat of number of packets processed */
    this->mStats[PS_PACKET_RECEIVED]++;
    this->mStats[PS_DAO_PACKET_RECEIVED]++;

    /* validate this packet, if possible */
    bool secure = this->check_security(dao, dao_len);

    rpl_node *peer;

    /* find the node entry from this source IP, and update seen time */
    /* this will create the node if it does not already exist! */
    if((peer = this->update_child(iface, from, ip6_to, now)) == NULL) {
        return;
    }

    if(mActive == false) {
	this->mStats[PS_PACKETS_WATCHED]++;
	return;
    }

    /* look for the suboptions, process them */
    rpl_dao decoded_dao(data, dao_len, this);
    unsigned int addrcount = 0;

    struct rpl_dao_target *rpltarget;
    while((rpltarget = decoded_dao.rpltarget()) != NULL) {
        char addrfound[SUBNETTOT_BUF];
        unsigned char v6bytes[16];
        int prefixbytes = ((rpltarget->rpl_dao_prefixlen+7) / 8);
        ip_subnet prefix;
        prefix.maskbits = rpltarget->rpl_dao_prefixlen;
        memset(v6bytes, 0, 16);
        memcpy(v6bytes, rpltarget->rpl_dao_prefix, prefixbytes);
        initaddr(v6bytes, 16, AF_INET6, &prefix.addr);

        addrcount++;

        subnettot(&prefix, 0, addrfound, sizeof(addrfound));


        /* need to look at dag_members, and see if the child node already
         * exists, and add if not
         */
        debug->verbose("  recv DAO rpltarget re: network %s, target %s (added)\n",
                       addrfound, peer->node_name());

        add_childnode(peer, iface, prefix);
    }
    maybe_send_dao();

    /* now send a DAO-ACK back this the node, using the interface it arrived on, if asked to. */
    if(RPL_DAO_K(dao->rpl_flags)) {
        debug->verbose("sending DAOACK about %u networks, to %s\n",
                       addrcount, peer->node_name());
        iface->send_daoack(*peer, *this, dao->rpl_daoseq);
    }

    /* increment stat of number of packets processed */
    this->mStats[PS_PACKET_PROCESSED]++;
}

void dag_network::receive_daoack(network_interface *iface,
                                 struct in6_addr from,
                                 struct in6_addr ip6_to,
                                 const time_t    now,
                                 const struct nd_rpl_daoack *daoack,
                                 unsigned char *data, int dao_len)
{
    /* validate the DAOACK came from the correct parent */
    if(dag_bestparent==NULL || !dag_bestparent->is_equal(from)) {
        debug->warn("received DAOACK from non-potential parent");
        this->mStats[PS_DAOACK_WRONG_PARENT]++;
        return;
    }

    /* check DAOSequence number */
    if(daoack->rpl_daoseq != mDAOSequence){
    	debug->warn("received DAOACK with incorrect sequence number, got %u, expected: %u",
                    daoack->rpl_daoseq, mDAOSequence);
        this->mStats[PS_DAOACK_WRONG_SEQNO]++;
    	return;
    }

    /* having got the message back, and validated it.. commit to this parent */
    commit_parent();
}

void dag_network::commit_parent(void)
{
    /*
     * when we commit to a parent, then we send all traffic to the prefix
     * that this parent announced towards the parent.
     */

    if(dag_bestparentif != dag_parentif ||
       dag_bestparent   != dag_parent) {
        /* potentially, there could have been multiple DESTINATION OPTIONS */

        dag_lastparent=dag_parent;
        dag_parentif = dag_bestparentif;
        dag_parent   = dag_bestparent;

        bool have_any_prefixes = this->dag_announced.size() > 0;
        if(have_any_prefixes) {
            prefix_node &srcip = this->dag_announced.begin()->second;

            dag_parentif->add_parent_route_to_prefix(mPrefix,
                                                     &srcip.prefix_number().addr,
                                                     *dag_parent);
            if(this->groundedP()) {
                dag_parentif->add_parent_route_to_default(&srcip.prefix_number().addr,
                                                          *dag_parent);
            }

        } else {
            dag_parentif->add_parent_route_to_prefix(mPrefix,
                                                     NULL,
                                                     *dag_parent);
            if(this->groundedP()) {
                dag_parentif->add_parent_route_to_default(NULL, *dag_parent);
            }
        }

        /* now send a DIO */
        maybe_schedule_dio();
    } else {
        debug->verbose("  already associated with this parent\n");
    }
}


void dag_network::set_interface_wildcard(const char *wild)
{
    if(mIfWildcard_max < DAG_IFWILDCARD_MAX) {
        int len = strlen(wild)+1;
        if(len > DAG_IFWILDCARD_LEN-1) len = DAG_IFWILDCARD_LEN;

        memcpy(mIfWildcard[mIfWildcard_max], wild, len);
        mIfWildcard[mIfWildcard_max][len]='\0';
        mIfWildcard_max++;
    }
}

void dag_network::set_interface_filter(const ip_subnet i6)
{
    if(mIfFilter_max < DAG_IFWILDCARD_MAX) {
        mIfFilter[mIfFilter_max++] = i6;
    }
}

void dag_network::set_interface_filter(const char *filter)
{
    ip_subnet i6;
    ttosubnet(filter, 0, AF_INET6, &i6);
    set_interface_filter(i6);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */

