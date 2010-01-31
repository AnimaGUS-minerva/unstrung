
typedef u_int8_t dagid_t[DAGID_LEN];

enum discard_reason {
        DR_SEQOLD,
        DR_MAX,
};

class dag_network {
public:
        dag_network(dagid_t dagid);
        ~dag_network();
        static class dag_network *find_by_dagid(dagid_t dagid);
        static class dag_network *find_or_make_by_dagid(dagid_t dagid);

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
        dagid_t mDagid;
        unsigned int mLastSeq;
        bool seq_too_old(unsigned int seq);
        bool upd_seq(unsigned int seq);
        u_int8_t last_seq() { return(mLastSeq & 0xff); };

        /* STUPID ME: NEED TO USE A LIST TYPE */
        void add_to_list(void) {
                this->next = dag_network::all_dag;
                dag_network::all_dag = this;
        };
        void remove_from_list(void);

        void receive_dio(const struct nd_rpl_dio *dio, int dio_len);

        /* let stats be public */
        u_int32_t mDiscards[DR_MAX];
                
private:
        void discard_dio(enum discard_reason dr);
        bool check_security(const struct nd_rpl_dio *dio,
                                         int dio_len);
        void seq_update(unsigned int seq);

        static const char *discard_reasons[DR_MAX+1];

        class dag_network *next;
        static class dag_network *all_dag;
};
