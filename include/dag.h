#ifndef _UNSTRUNG_DAG_H_
#define _UNSTRUNG_DAG_H_

#include "node.h"

typedef u_int8_t dagid_t[DAGID_LEN];

enum packet_stats {
        PS_SEQ_OLD,
        PS_PACKET_RECEIVED,
        PS_PACKET_PROCESSED,
        PS_LOWER_RANK_CONSIDERED,
        PS_LOWER_RANK_REJECTED,
        PS_MAX,
};

class dag_network {
public:
        dag_network(dagid_t dagid);
        ~dag_network();
        static class dag_network *find_by_dagid(dagid_t dagid);
        static class dag_network *find_or_make_by_dagid(dagid_t dagid,
                                                        bool verbose_flag,
                                                        FILE *verbose_file);

        int cmp_dag(dagid_t n_dagid) {
                return memcmp(mDagid, n_dagid, DAGID_LEN);
        };

        /* debugging -- refactor me */
	bool                    verbose_flag;
	FILE                   *verbose_file;
        bool                    verbose_test() {
                return(verbose_flag && verbose_file!=NULL);
        };
	void set_verbose(bool flag) { verbose_flag = flag; }
	void set_verbose(bool flag, FILE *out) {
		verbose_flag = flag;
		verbose_file = out;
	}
	int  verboseprint() { return verbose_flag; }

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

        void receive_dio(struct in6_addr from,
                         const time_t    now,
                         const struct nd_rpl_dio *dio, int dio_len);
        void potentially_lower_rank(rpl_node peer,
                                    const struct nd_rpl_dio *dio, int dio_len);

        int  member_count() { return dag_members.size(); };
        bool contains_member(const struct in6_addr member) {
            return(get_member(member) != NULL);
        };
        rpl_node *get_member(const struct in6_addr member);
        
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

