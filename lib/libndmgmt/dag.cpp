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
#include "oswlibs.h"
#include "rpl.h"

}

#include "iface.h"
#include "dag.h"
#include "dio.h"
#include "dao.h"

class dag_network *dag_network::all_dag = NULL;
u_int32_t dag_network::globalStats[PS_MAX];

unsigned char           dag_network::optbuff[256];
unsigned int            dag_network::optlen;


void dag_network::format_dagid(char *dagidstr,
			       const u_int8_t rpl_dagid[DAGID_LEN])
{
    char *d = dagidstr;
    bool lastwasnull=false;
    int  i;

    /* should attempt to format as IPv6 address too */
    for(i=0;i<16;i++) {
        if(isprint(rpl_dagid[i])) {
            *d++ = rpl_dagid[i];
            lastwasnull=false;
        } else {
            if(rpl_dagid[i] != '\0' || !lastwasnull) {
                int cnt=snprintf(d, 6,"0x%02x ", rpl_dagid[i]);
                d += strlen(d);
            }
            lastwasnull = (rpl_dagid[i] == '\0');
        }
    }
    *d++ = '\0';
}

void dag_network::init_stats(void)
{
}

void dag_network::init_dag_name(void)
{
    memset(mDagName, 0, sizeof(mDagName));
    char *dn = mDagName;
    for(int i=0; i<DAGID_LEN && dn < mDagName+sizeof(mDagName)-2; i++) {
	int c = mDagid[i];
	if(c > ' ' && c < 127) {
	    *dn++ = c;
	} else if(c==0) {
	    /* nothing */
	} else {
	    sprintf(dn, "%02x", c);
	    dn += 2;
	}
    }
    *dn='\0';
}

void dag_network::init_dag(void)
{
    mLastSeq = 0;
    mMyRank   = UINT_MAX;
    mBestRank = UINT_MAX;
    mSequence = INVALID_SEQUENCE;
    mInstanceid = 1;
    mVersion  = 1;
    debug     = NULL;
    memset(mStats,     0, sizeof(mStats));
    memset(old_mStats, 0, sizeof(old_mStats));

    init_dag_name();
    this->add_to_list();
    dag_me.set_debug(debug);
}

dag_network::dag_network(dagid_t n_dagid, rpl_debug *deb)
{
    set_dagid(n_dagid);
    init_dag();
    set_debug(deb);
}


dag_network::dag_network(const char *s_dagid, rpl_debug *deb)
{
    set_dagid(s_dagid);
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

    dag_me.set_prefix(mPrefix);
}

void dag_network::set_grounded(bool grounded)
{
    if(grounded) {
        this->mGrounded = true;

        /* now add this prefix as a blackhole route on lo */
        if(loopback_interface)
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


class dag_network *dag_network::find_or_make_by_dagid(dagid_t n_dagid,
                                                      rpl_debug *debug,
						      bool watching)
{
        class dag_network *dn = find_by_dagid(n_dagid);

        if(dn==NULL) {
            dn = new dag_network(n_dagid, debug);
            if(watching) {
                globalStats[PS_DAG_CREATED_FOR_WATCHING]++;
            }
            dn->set_inactive();             /* in active by default */
        }
        return dn;
}

class dag_network *dag_network::find_or_make_by_string(const char *dagid,
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
    return find_or_make_by_dagid(d, debug, watching);
}

class dag_network *dag_network::find_by_dagid(dagid_t n_dagid)
{
        class dag_network *dn = dag_network::all_dag;

        while(dn != NULL &&
              dn->cmp_dag(n_dagid)!=0) {
                dn = dn->next;
        }
        return dn;
}

void dag_network::print_stats(FILE *out, const char *prefix)
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

void dag_network::print_all_dagstats(FILE *out, const char *prefix)
{
    char prefixbuf2[64];

    for(class dag_network *dn = dag_network::all_dag;
	dn != NULL;
	dn = dn->next) {
	snprintf(prefixbuf2, sizeof(prefixbuf2), "%s(%s) ",
		 prefix, dn->get_dagName());
	dn->print_stats(out, prefixbuf2);
    }
}

void dag_network::discard_dio(enum packet_stats dr)
{
    mStats[dr]++;

    if(VERBOSE(this))
        fprintf(this->verbose_file, "  DIO discarded %s (++%u)\n", dag_network_packet_stat_names[dr]);
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
    mTimeToSendDao=true;
}

/* here we mark that a DIO is needed soon */
void dag_network::maybe_send_dio(void)
{
    mTimeToSendDio=true;
}

void dag_network::add_childnode(rpl_node announcing_peer,
                                  network_interface *iface,
                                  ip_subnet prefix)
{
    prefix_node &pre = this->dag_children[prefix];

    if(!pre.is_installed()) {
        pre.set_debug(this->debug);
        pre.set_announcer(&announcing_peer);
        maybe_send_dao();
    }
}

void dag_network::add_prefix(rpl_node advertising_peer,
                             network_interface *iface,
                             ip_subnet prefix)
{
    prefix_node &pre = this->dag_children[prefix];

    if(!pre.is_installed()) {
        this->set_prefix(prefix);
        pre.set_prefix(prefix);
        pre.set_debug(this->debug);
        pre.set_announcer(&advertising_peer);
        pre.configureip(iface, this);

        maybe_send_dio();
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
 */
void dag_network::addselfprefix(network_interface *iface)
{
    ip_subnet               ll_prefix;

    rpl_node *me = find_or_make_member(iface->if_addr);
    me->makevalid(iface->if_addr, this, this->debug);
    me->markself(iface->get_if_index());

    /* update the prefix_node presenting ourselves in this dag */
    dag_me.update_announcer(iface->host_node());
    dag_me.configureip(iface, this);

#if 0
    err_t blah = initsubnet(&me->node_address(), 128, '0', &ll_prefix);
    if(blah) {
        debug->verbose("initsubnet says: %s\n", blah);
    }
    ll_prefix.maskbits = 128;

    /* insert self into dag */
    prefix_node &pre = this->dag_children[ll_prefix];

    pre.markself(this, ll_prefix);
    pre.linklocal = true;
#endif
}

static int addselfprefix_each(network_interface *iface, void *arg)
{
    dag_network *that = (dag_network *)arg;
    if(!iface->loopbackP()) {
        that->addselfprefix(iface);
    }
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

    if(dag_parent == &peer) {
	debug->verbose("  But it is the same parent as before: ignored\n");
        this->mStats[PS_SAME_PARENT_IGNORED]++;
	return;
    }
    /* XXX
     * this is actually quite a big deal (SEE rfc6550), setting my RANK.
     * just fake it for now by adding 1.
     */
    if(mSequence != INVALID_SEQUENCE && mSequence >= dio->rpl_dtsn) {
	debug->verbose("  Same sequence number, ignore\n");
        this->mStats[PS_SAME_SEQUENCE_IGNORED]++;
	return;
    }

    mSequence     = dio->rpl_dtsn;
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

    dag_parentif = iface;
    dag_parent   = &peer;

    /* now see if we have already an address on this new network */
    /*
     * to do this, we have to crack open the DIO.  UP to this point
     * we haven't taken the DIO apart, so do, keeping stuff on the stack.
     */
    rpl_dio decoded_dio(peer, dio, dio_len);

    struct rpl_dio_destprefix *dp;
    while((dp = decoded_dio.destprefix()) != NULL) {
        unsigned char v6bytes[16];
        int prefixbytes = ((dp->rpl_dio_prefixlen+7) / 8);
        ip_subnet prefix;
        prefix.maskbits = dp->rpl_dio_prefixlen;
        memset(v6bytes, 0, 16);
        memcpy(v6bytes, dp->rpl_dio_prefix, prefixbytes);
        initaddr(v6bytes, 16, AF_INET6, &prefix.addr);

        add_prefix(peer, iface, prefix);
    }

    /* now schedule sending out packets */
    schedule_dio();
    schedule_dao();
}

/*
 * send out outgoing DAO
 */
void dag_network::send_dao(void)
{
    int cnt = 0;
    prefix_map_iterator pi = dag_children.begin();
    while(pi != dag_children.end()) {
	prefix_node &pm = pi->second;

        debug->verbose("SENDING[%u] dao about %s for %s to: %s on if=%s\n",
                       cnt, pm.node_name(),
                       mDagName, dag_parent->node_name(),
                       dag_parentif ? dag_parentif->get_if_name():"unknown");
        pi++;
        cnt++;
    }

    if(dag_parent == NULL || dag_parentif ==NULL) return;

    /* need to tell our parent about how to reach us */
    dag_parentif->send_dao(*dag_parent, *this);
}

/*
 * this routine needs to send out a DIO sooner than
 * we would otherwise.
 */
void dag_network::schedule_dio(void)
{
    schedule_dio(mInterval_msec);
}

void dag_network::schedule_dio(unsigned int msec)
{
    /* do nothing if there is no valid DAG */
    if(dag_rank_infinite()) return;

    debug->verbose("Scheduling dio in %u ms\n", msec+1);

    if(!mSendDioEvent) {
	mSendDioEvent = new rpl_event(0, mInterval_msec,
                                      rpl_event::rpl_send_dio,
				      mDagName, this->debug);
    }
    mSendDioEvent->event_type = rpl_event::rpl_send_dio;
    mSendDioEvent->reset_alarm(0, msec+1);

    mSendDioEvent->mDag = this;
    mSendDioEvent->requeue();
    //this->dio_event = re;        // needs to be smart-pointer

}

/*
 * this routine needs to send out a DAO sooner than
 * we would otherwise.
 */
void dag_network::schedule_dao(void)
{
    debug->verbose("Scheduling dao in %u ms\n", mInterval_msec+2);

    if(!mSendDaoEvent) {
	mSendDaoEvent = new rpl_event(0, mInterval_msec+2, rpl_event::rpl_send_dao,
				      mDagName, this->debug);
    }

    mSendDaoEvent->event_type = rpl_event::rpl_send_dao;
    mSendDaoEvent->reset_alarm(0, mInterval_msec+2);

    mSendDaoEvent->mDag = this;
    mSendDaoEvent->requeue();
}

rpl_node *dag_network::update_route(network_interface *iface,
				    ip_subnet &prefix, const time_t now)
{
    struct in6_addr *from = &prefix.addr.u.v6.sin6_addr;

    rpl_node &peer = this->dag_members[*from];
    peer.makevalid(*from, this, this->debug);

    peer.set_last_seen(now);
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

rpl_node *dag_network::update_node(network_interface *iface,
                                   struct in6_addr from,
                                   struct in6_addr ip6_to,
                                   const time_t now)
{
    rpl_node &peer = this->dag_members[from];

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
    char dagid[16*6];

    format_dagid(dagid, dio->rpl_dagid);
    debug->info_more(" [seq:%u,instance:%u,rank:%u,version:%u,%s%s,dagid:%s]\n",
                dio->rpl_dtsn, dio->rpl_instanceid, ntohs(dio->rpl_dagrank),
                dio->rpl_version,
                (RPL_DIO_GROUNDED(dio->rpl_mopprf) ? "grounded," : ""),
                dag_network::mop_decode(dag_network::mop_extract(dio)),
                dagid);
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

    if(this->seq_too_old(dio->rpl_dtsn)) {
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

    debug->info(" processing dio(%u) ",dio_len);  /* \n will be below */
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
    } else {
        int len = strlen(dagstr);
        if(len > 16) len=16;

        memset(this->mDagid, 0, 16);
        memcpy(this->mDagid, dagstr, len);
    }
    init_dag_name();
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
    int dao_payload_len = dao_len - sizeof(*dao);

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
    rpl_dao decoded_dao(data, dao_payload_len);

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

        subnettot(&prefix, 0, addrfound, sizeof(addrfound));

        debug->verbose("received DAO about network %s, target %s\n", addrfound, peer->node_name());

	iface->add_route_to_node(prefix, peer, dag_me.prefix_number().addr);
    }

    /* now send a DAO-ACK back this the node, if asked to. */
    if(RPL_DAO_K(dao->rpl_flags)) {

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
    /* XXX */
}



/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make programs"
 * End:
 */

