#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
const char *dag_network::discard_reasons[DR_MAX+1]={
    "sequence too old",
    "max reason"
};

void dag_network::discard_dio(enum discard_reason dr)
{
    mDiscards[dr]++;

    if(VERBOSE(this))
        fprintf(this->verbose_file, "  DIO discarded %s (++%u)\n", discard_reasons[dr]);
}

/*
 * sequence numbers are kept as 32-bit integers, but when
 * transmitted, only the lower 8 bits matters, so they wrap
 */
bool dag_network::seq_too_old(unsigned int seq)
{
    if(seq < mLastSeq) {
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
    if(seq > mLastSeq) {
        mLastSeq = seq;
    }
}

bool dag_network::check_security(const struct nd_rpl_dio *dio, int dio_len)
{
    /* XXXX */
    return true;
}

/*
 * Process an incoming DIO. 
 *
 */
void dag_network::receive_dio(const struct nd_rpl_dio *dio, int dio_len)
{
    /* it has already been checked to be at least sizeof(*dio) */
    int dio_payload_len = dio_len - sizeof(*dio);

    if(this->seq_too_old(dio->rpl_seq)) {
        this->discard_dio(DR_SEQOLD);
        return;
    }

    /* validate this packet */
    this->check_security(dio, dio_len);

    this->seq_update(dio->rpl_seq);

    //if(dio->rpl_
}



/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

