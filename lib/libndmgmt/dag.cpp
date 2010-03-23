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

class dag_network *dag_network::all_dag = NULL;

dag_network::dag_network(dagid_t n_dagid)
{
        memcpy(mDagid, n_dagid, DAGID_LEN);
        mLastSeq = 0;
        mDagRank = UINT_MAX;

        this->add_to_list();
        
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


class dag_network *dag_network::find_or_make_by_dagid(dagid_t n_dagid)
{
        class dag_network *dn = find_by_dagid(n_dagid);
        
        if(dn==NULL) {
                dn = new dag_network(n_dagid);
        }
        return dn;
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
    "packets receied"
    "packets processed",
    "packets with <dagrank",
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

void dag_network::potentially_lower_rank(rpl_node peer,
                                         const struct nd_rpl_dio *dio,
                                         int dio_len)
{
    if(VERBOSE(this))
        fprintf(this->verbose_file, "  DIO has better rank %u < %u\n",
                dio->rpl_dagrank, mDagRank);

    this->mStats[PS_LOWER_RANK_CONSIDERED]++;
}


/*
 * Process an incoming DIO. 
 *
 */
void dag_network::receive_dio(struct in6_addr from,
                              time_t          now,
                              const struct nd_rpl_dio *dio, int dio_len)
{
    /* it has already been checked to be at least sizeof(*dio) */
    int dio_payload_len = dio_len - sizeof(*dio);

    /* increment stat of number of packets processed */
    this->mStats[PS_PACKET_RECEIVED]++;

    if(this->seq_too_old(dio->rpl_seq)) {
        this->discard_dio(PS_SEQ_OLD);
        return;
    }

    /* validate this packet */
    this->check_security(dio, dio_len);

    /* find the node entry from this source IP, and update seen time */
    /* this will create the node if it does not already exist! */
    rpl_node peer  = this->dag_members[from];
    peer.set_last_seen(now);

    this->seq_update(dio->rpl_seq);

    if(dio->rpl_dagrank < mDagRank) {
        this->potentially_lower_rank(peer, dio, dio_len);
    }

    /* increment stat of number of packets processed */
    this->mStats[PS_PACKET_PROCESSED]++;
}

rpl_node *dag_network::get_member(const struct in6_addr memberaddr)
{
    node_map_iterator ni = dag_members.find(memberaddr);

    if(ni == dag_members.end()) return NULL;
    else return &ni->second;
}




/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

