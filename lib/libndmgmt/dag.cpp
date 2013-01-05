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
    mDagRank = UINT_MAX;
    memset(mStats, 0, sizeof(mStats));

    init_dag_name();
    this->add_to_list();
}

dag_network::dag_network(dagid_t n_dagid) 
{
    set_dagid(n_dagid);
    init_dag();
}


dag_network::dag_network(char *s_dagid)
{
    set_dagid(s_dagid);
    init_dag();
}

dag_network::~dag_network()
{
        this->remove_from_list();
}

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
                dn = new dag_network(n_dagid);
		if(watching) {
		    globalStats[PS_DAG_CREATED_FOR_WATCHING]++;
		}
		dn->set_inactive();             /* in active by default */
                dn->set_debug(debug);
        }
        return dn;
}

class dag_network *dag_network::find_or_make_by_string(const char *dagid,
						       rpl_debug *debug, 
						       bool watching)
{
    dagid_t d;
    memset(d, 0, sizeof(dagid_t));
    strncat((char *)d, dagid, sizeof(dagid_t));
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

/* provide a count of discards */
const char *dag_network::packet_stat_names[PS_MAX+1]={
    "sequence too old",
    "packets received"
    "packets processed",
    "packets received due to watch",
    "packets with <dagrank",
    "packets with <dagrank rejected",
    "packets where subopt was too short",
    "packets from self that were ignored",
    "DAO packets received",
    "DIO packets received",
    "DAO packets ignored (non-local DODAG id)",
    "DIO packets ignored (non-local DODAG id)",
    "DAG created due to watch",
    "max reason"
};

void dag_network::discard_dio(enum packet_stats dr)
{
    mStats[dr]++;

    if(VERBOSE(this))
        fprintf(this->verbose_file, "  DIO discarded %s (++%u)\n", packet_stat_names[dr]);
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

void dag_network::addprefix(rpl_node peer,
                            network_interface *iface,
                            ip_subnet prefix)
{
    prefix_node &pre = this->dag_children[prefix];

    if(!pre.is_installed()) {
        pre.set_debug(this->debug);
        pre.set_announcer(&peer);
        pre.set_dn(this);
        pre.set_prefix(prefix);
        pre.configureip(iface);
        maybe_send_dao();
    }
}

void dag_network::potentially_lower_rank(rpl_node &peer,
                                         network_interface *iface,
                                         const struct nd_rpl_dio *dio,
                                         int dio_len)
{
    unsigned int rank = ntohs(dio->rpl_dagrank);

    debug->verbose("  does peer '%s' have better rank? (%u < %u)\n",
                   peer.node_name(), rank, mDagRank);

    this->mStats[PS_LOWER_RANK_CONSIDERED]++;

    if(rank > mDagRank) {
        this->mStats[PS_LOWER_RANK_REJECTED]++;
        return;
    }

    debug->verbose("  Yes, '%s' has best rank %u\n",
                   peer.node_name(), rank);

    /* XXX
     * this is actually quite a big deal (SEE ID), setting my RANK.
     * just fake it for now by adding 1.
     */
    mDagRank     = rank + 1;
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

        addprefix(peer, iface, prefix);
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
    debug->verbose("SENDING dao for %s to: %s on if=%s\n",
                   mDagName, dag_parent->node_name(),
                   dag_parentif ? dag_parentif->get_if_name():"unknown");

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
    debug->verbose("Scheduling dio in %u ms\n", mInterval_msec);

    if(!mSendDioEvent) {
	mSendDioEvent = new rpl_event(0, mInterval_msec, rpl_event::rpl_send_dio,
				      mDagName, this->debug);
    } 
    mSendDioEvent->event_type = rpl_event::rpl_send_dio;
    mSendDioEvent->reset_alarm(0, mInterval_msec);

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
    debug->verbose("Scheduling dao in %u ms\n", mInterval_msec);

    if(!mSendDaoEvent) {
	mSendDaoEvent = new rpl_event(0, mInterval_msec, rpl_event::rpl_send_dao,
				      mDagName, this->debug);
    } 
	
    mSendDaoEvent->event_type = rpl_event::rpl_send_dao;
    mSendDaoEvent->reset_alarm(0, mInterval_msec);

    mSendDaoEvent->mDag = this;
    mSendDaoEvent->requeue();
}    


rpl_node *dag_network::update_origin(network_interface *iface,
                                     struct in6_addr from, const time_t now)
{
    rpl_node &peer = this->dag_members[from];

    if(peer.isself() ||
       iface->if_ifaddr(from)) {
        peer.markself(iface->get_if_index());

        this->mStats[PS_SELF_PACKET_RECEIVED]++;
        debug->info("  received self packet (%u/%u)\n",
                    this->mStats[PS_SELF_PACKET_RECEIVED],
                    this->mStats[PS_PACKET_RECEIVED]);
        return NULL;
    }

    peer.makevalid(from, this, this->debug);
    peer.set_last_seen(now);

    return &peer;
}

/*
 * Process an incoming DODAG Information Object (DIO)
 * the DIO is the downward announcement.
 *
 */
void dag_network::receive_dio(network_interface *iface,
                              struct in6_addr from,
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
    if((peer = this->update_origin(iface, from, now)) == NULL) {
        return;
    }

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
    if((peer = this->update_origin(iface, from, now)) == NULL) {
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

        debug->verbose("received DAO about network %s\n", addrfound);
    }

    /* increment stat of number of packets processed */
    this->mStats[PS_PACKET_PROCESSED]++;
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

