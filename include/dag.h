#ifndef _UNSTRUNG_DAG_H_
#define _UNSTRUNG_DAG_H_

#include "rpl.h"
#include "node.h"
#include "prefix.h"
#include "debug.h"

typedef u_int8_t dagid_t[DAGID_LEN];

enum packet_stats {
        PS_SEQ_OLD,
        PS_PACKET_RECEIVED,
        PS_PACKET_PROCESSED,
        PS_LOWER_RANK_CONSIDERED,
        PS_LOWER_RANK_REJECTED,
        PS_SUBOPTION_UNDERRUN,
        PS_SELF_PACKET_RECEIVED,
        PS_MAX,
};

class rpl_dio;

class dag_network {
public:
        dag_network(dagid_t dagid);
        ~dag_network();
        static class dag_network *find_by_dagid(dagid_t dagid);
        static class dag_network *find_or_make_by_dagid(dagid_t dagid,
                                                        rpl_debug *debug);

        int cmp_dag(dagid_t n_dagid) {
                return memcmp(mDagid, n_dagid, DAGID_LEN);
        };

	void set_debug(rpl_debug *deb) { debug = deb; };
        rpl_debug        *debug;

        /* prime key for the DAG */
        dagid_t                    mDagid;

        unsigned int               mLastSeq;
        bool seq_too_old(unsigned int seq);
        bool upd_seq(unsigned int seq);
        u_int8_t last_seq() { return(mLastSeq & 0xff); };

        unsigned int               mDagRank;
        bool dag_rank_infinite(void) { return (mDagRank == UINT_MAX); };


        /* STUPID ME: NEED TO USE A LIST TYPE */
        void add_to_list(void) {
                this->next = dag_network::all_dag;
                dag_network::all_dag = this;
        };
        void remove_from_list(void);

        void receive_dio(network_interface *iface,
                         struct in6_addr from,
                         const time_t    now,
                         const struct nd_rpl_dio *dio, int dio_len);
        void addprefix(rpl_node peer,
                       network_interface *iface,
                       ip_subnet prefix);
        unsigned int prefixcount(void) {
            return dag_prefixes.size();
        };
        void potentially_lower_rank(rpl_node peer,
                                    network_interface *iface,
                                    const struct nd_rpl_dio *dio, int dio_len);

        int  member_count() { return dag_members.size(); };
        bool contains_member(const struct in6_addr member) {
            return(get_member(member) != NULL);
        };
        rpl_node *get_member(const struct in6_addr member);
        rpl_node *find_or_make_member(const struct in6_addr memberaddr);
        
        /* let stats be public */
        u_int32_t mStats[PS_MAX];
                
private:
        /* information about this DAG */
        
        void discard_dio(enum packet_stats dr);
        bool check_security(const struct nd_rpl_dio *dio,
                                         int dio_len);
        void seq_update(unsigned int seq);

        static const char *packet_stat_names[PS_MAX+1];

        node_map           dag_members;
        prefix_map         dag_prefixes;     /* usually only one */

        // XXX replace with dag_network_map!!!
        class dag_network *next;
        static class dag_network *all_dag;
};

#endif /* _UNSTRUNG_DAG_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

