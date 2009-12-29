
typedef u_int8_t dagid_t[DAGID_LEN];

class dag_network {
public:
        dag_network(dagid_t dagid);
        static class dag_network *find_by_dagid(dagid_t dagid);

        int cmp_dag(dagid_t n_dagid) {
                return memcmp(mDagid, n_dagid, DAGID_LEN);
        };

        /* prime key for the DAG */
        dagid_t mDagid;
        

private:
        class dag_network *next;
        static class dag_network *all_dag;
};
