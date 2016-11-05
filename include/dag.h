#ifndef _UNSTRUNG_DAG_H_
#define _UNSTRUNG_DAG_H_

#include "rpl.h"
#include "node.h"
#include "prefix.h"
#include "debug.h"
extern "C" {
    /* for packet_stats enum */
#include "counter_names.h"
}

typedef u_int8_t   dagid_t[DAGID_LEN];
typedef u_int16_t  instanceID_t;

#define RANK_INFINITE     65537     /* rank is otherwise 16bits */
#define INVALID_SEQUENCE    257     /* sequence is 8 bits */

class rpl_dio;
class rpl_event;

class dag_network {
public:
    dag_network(instanceID_t instanceID, dagid_t dagid, rpl_debug *debug);
    dag_network(instanceID_t instanceID, struct in6_addr *dagnum, rpl_debug *debug);
    dag_network(instanceID_t instanceID, const char *dagid, rpl_debug *debug);
    dag_network(instanceID_t instanceID, rpl_debug *debug);
    ~dag_network();
    static class dag_network *find_by_instanceid(instanceID_t num, dagid_t dagid);
    static class dag_network *find_by_instanceid(instanceID_t num);
    static class dag_network *find_or_make_by_instanceid(instanceID_t num,
                                                    dagid_t dagid,
                                                    rpl_debug *debug,
                                                    bool watching);
    static class dag_network *find_or_make_by_instanceid(instanceID_t num,
                                                    struct in6_addr dagid,
                                                    rpl_debug *debug,
                                                    bool watching);
    static class dag_network *find_or_make_by_string(instanceID_t num,
                                                     const char *dagid,
                                                     rpl_debug *debug,
                                                     bool watching);
        static void init_stats(void);

        int cmp_dag(dagid_t n_dagid) {
                return memcmp(mDagid, n_dagid, DAGID_LEN);
        };

	void set_debug(rpl_debug *deb) {
            debug = deb;
            if(dag_me) {
                dag_me->set_debug(deb);
            }
        };
	void set_active()   { mActive = true; };
	void set_inactive() { mActive = false;};

        rpl_debug        *debug;

        /* prime key for the DAG */
        dagid_t                    mDagid;
	bool                       mActive;
	bool 	                   mIgnorePio;
	bool                       mPrefixSet;

        unsigned int               mLastSeq;
        bool seq_too_old(unsigned int seq);
        bool upd_seq(unsigned int seq);
        u_int8_t last_seq() { return(mLastSeq & 0xff); };

        /* might depend if mGrounded too */
        bool root_node(void) { return mMyRank == 1; };

        unsigned int               mMyRank;     /* my rank */
        unsigned int               mBestRank;   /* my best parent */
        bool dag_rank_infinite(void) { return (mBestRank >= RANK_INFINITE); };

        /* this manages and evaluates a list of interface wildcards,
         * and prefix (CIDR) patterns for addresses that will be added
         * as locale addresses for DAOs sent from this node.
         */
        bool set_interface_wildcard(const char *ifname);
        bool set_interface_filter(const char *filter);
        bool set_interface_filter(const ip_subnet i6);
        bool matchesIfWildcard(const char *ifname);
        bool matchesIfPrefix(const ip_address v6);
        bool matchesIfPrefix(const struct in6_addr v6);
        prefix_node *add_address(const ip_subnet v6);
        prefix_node *add_address(const ip_address v6);
        static bool notify_new_interface(network_interface *one);

        void set_prefix(const struct in6_addr v6, const int prefixlen);
        void set_prefix(const ip_subnet prefix);
        const char *prefix_name(void);
        const ip_subnet get_prefix(void) { return mPrefix; };

        /* STUPID ME: NEED TO USE A LIST TYPE */
        void add_to_list(void) {
                this->next = dag_network::all_dag;
                dag_network::all_dag = this;
        };
        void remove_from_list(void);

        void receive_dis(network_interface *iface,
                         struct in6_addr from,
                         struct in6_addr ip6_to,
                         const time_t    now,
                         const struct nd_rpl_dis *dis, int dis_len);

        void receive_dio(network_interface *iface,
                         struct in6_addr from,
                         struct in6_addr ip6_to,
                         const time_t    now,
                         const struct nd_rpl_dio *dio, int dio_len);

        void receive_dao(network_interface *iface,
                         struct in6_addr from,
                         struct in6_addr ip6_to,
                         const time_t    now,
                         const struct nd_rpl_dao *dao,
                         unsigned char *data, int data_len);

        void receive_daoack(network_interface *iface,
                            struct in6_addr from,
                            struct in6_addr ip6_to,
                            const time_t    now,
                            const struct nd_rpl_daoack *daoack,
                            unsigned char *data, int data_len);

        void add_childnode(rpl_node *peer,
                           network_interface *iface,
                           ip_subnet prefix);
        void add_prefix(rpl_node peer,
                           network_interface *iface,
                           ip_subnet prefix);
	void addselfprefix(network_interface *iface);
        unsigned int prefixcount(void) {
            return dag_prefixes.size();
        };
	void maybe_send_dao(void);
	void maybe_schedule_dio(void);
        void potentially_lower_rank(rpl_node &peer,
                                    network_interface *iface,
                                    const struct nd_rpl_dio *dio, int dio_len);

        int  member_count() { return dag_members.size(); };
        bool contains_member(const struct in6_addr member) {
            return(get_member(member) != NULL);
        };
        rpl_node *get_member(const struct in6_addr member);
        rpl_node *find_or_make_member(const struct in6_addr memberaddr);

        /* send a unicast summary to new parent */
        void send_dao(void);
        void schedule_dio(void);
        void schedule_dio(unsigned int when);

	/* send out a new downstream announcement */
        void schedule_dao(void);
        void clear_event(rpl_event *thisone);

        /* let stats be public */
	u_int32_t old_mStats[PS_MAX];
        u_int32_t mStats[PS_MAX];
	void print_stats(FILE *out, const char *prefix);
	static void print_all_dagstats(FILE *out, const char *prefix);

        /* let global stats public */
        static u_int32_t globalStats[PS_MAX];
        static u_int32_t globalOldStats[PS_MAX];

        /* some decode routines */
        static const char *mop_decode(unsigned int mop) {
            switch(mop) {
            case RPL_DIO_NO_DOWNWARD_ROUTES_MAINT: return "no-downward-route-maint";
            case RPL_DIO_NONSTORING:    return "non-storing";
            case RPL_DIO_STORING_NO_MULTICAST: return "storing-no-mcase";
            case RPL_DIO_STORING_MULTICAST:    return "storing-mcast";
            case 4:                  return "unknown-mop4";
            case 5:                  return "unknown-mop5";
            case 6:                  return "unknown-mop6";
            case 7:                  return "unknown-mop7";
            }
        };

	static void format_dagid(char *dagidstr,
                                 unsigned int   dagidstr_len,
                                 instanceID_t   rpl_instanceId,
				 const u_int8_t rpl_dagid[DAGID_LEN]);
	static void dump_dio(rpl_debug *debug, const struct nd_rpl_dio *dio);

        static const unsigned int mop_extract(const struct nd_rpl_dio *dio) {
            return RPL_DIO_MOP(dio->rpl_mopprf);
        };

	const char *get_dagName(void) {
	    return mDagName;
	};
	void set_dagid(dagid_t dagid);
	void set_dagid(const char *dagstr);
	void set_dagid(struct in6_addr addr) {
            dagid_t *nDagId = (dagid_t *)addr.s6_addr;
            set_dagid(*nDagId);
        };
	unsigned int get_dagRank(void) {
	    return mMyRank;
	};
	void set_ignore_pio(const bool ignore) {
            mIgnorePio = ignore;
	};
	void set_dagrank(const unsigned int dagrank) {
	    mMyRank   = dagrank;
	    mBestRank = dagrank;
	};
	void set_sequence(const unsigned int sequence) {
		mDTSN = sequence;
	};
	void set_instanceid(const unsigned int instanceid) {
	    mInstanceid = instanceid;
	};
        instanceID_t get_instanceid(void) { return mInstanceid; };
	void set_prefixlifetime(const unsigned int lifetime) {
	    mLifetime = lifetime;
	};
	void set_version(const unsigned int version) {
	    mVersion = version;
	};
	void set_grounded(const bool grounded);
        bool groundedP(void) { return mGrounded; };
	void set_interval(const int msec) {
	    mInterval_msec = msec;
	};
        void add_all_interfaces(void);
	rpl_node    *my_dag_node(void);
	dag_network *my_dag_net(void);

	void set_mode(enum RPL_DIO_MOP m) {
	    mMode = m;
	};

	void set_nomulticast() {
	    if(mMode == RPL_DIO_STORING_MULTICAST) {
		mMode = RPL_DIO_STORING_NO_MULTICAST;
	    }
	}

	void set_multicast() {
	    if(mMode == RPL_DIO_STORING_NO_MULTICAST) {
		mMode = RPL_DIO_STORING_MULTICAST;
	    }
	}

	/* public for now, need better inteface */
        prefix_map         dag_children;     /* list of addresses downstream, usually /128 */
        prefix_map         dag_prefixes;     /* list of addresses, by prefix in this dag */
        prefix_map         dag_announced;    /* list of me, to announce upstream  */
        bool               dao_needed;
        void               set_dao_needed() { dao_needed = true; };
        prefix_node       *dag_me;           /* my identity in this dag (/128) */

	int build_info_disopt(void);
	int build_prefix_dioopt(ip_subnet prefix);
        int build_target_opt(struct in6_addr addr, int maskbits);
	int build_target_opt(ip_subnet prefix);

	int  build_dis(unsigned char *buff, unsigned int buff_len);
	int  build_dio(unsigned char *buff, unsigned int buff_len, ip_subnet prefix);
	int  build_dao(unsigned char *buff, unsigned int buff_len);
	int  build_daoack(unsigned char *buff, unsigned int buff_len, unsigned short seq_num);

	/* should be private */
	ip_subnet               mPrefix;
	char                    mPrefixName[SUBNETTOT_BUF];

private:
        dag_network(void);
	void init_dag(void);
	static unsigned char           optbuff[256];
	static unsigned int            optlen;

#define DAG_IFWILDCARD_MAX 8
#define DAG_IFWILDCARD_LEN 32
        int                        mIfWildcard_max;
        char                       mIfWildcard[DAG_IFWILDCARD_MAX][DAG_IFWILDCARD_LEN];
        int                        mIfFilter_max;
        ip_subnet                  mIfFilter[DAG_IFWILDCARD_MAX];

    /* space to format various messages */
    int append_suboption(unsigned char *buff,
                             unsigned int buff_len,
                             enum RPL_SUBOPT subopt_type,
                             unsigned char *subopt_data,
                             unsigned int subopt_len);
    int append_suboption(unsigned char *buff,
                             unsigned int buff_len,
                             enum RPL_SUBOPT subopt_type);

        /* information about this DAG */

        void discard_dio(enum packet_stats dr);
        bool check_security(const struct nd_rpl_dio *dio,
                                         int dio_len);
        bool check_security(const struct nd_rpl_dao *dao,
                                         int dao_len);

        rpl_node *update_node(network_interface *iface,
			      struct in6_addr from,
			      struct in6_addr ip6_to,
			      const time_t now);

        rpl_node *update_child(network_interface *iface,
			       struct in6_addr from,
                               struct in6_addr ip6_to,
			       const time_t now);
        rpl_node *update_parent(network_interface *iface,
                                struct in6_addr from,
                                struct in6_addr ip6_to,
                                const time_t now);
        rpl_node *update_route(network_interface *iface,
			       ip_subnet &prefix,
			       const time_t now);
        void seq_update(unsigned int seq);
	void init_dag_name(void);
        void commit_parent(void);

        static const char *packet_stat_names[PS_MAX+1];

        node_map           dag_members;      /* list of dag members, by link-layer address */
        rpl_node          *dag_parent;       /* current parent (shared_ptr XXX) */
        network_interface *dag_parentif;     /* how to get to parent, shared_ptr */
        rpl_node          *dag_lastparent;   /* previous parent */
        rpl_node          *dag_bestparent;   /* running best parent */
        network_interface *dag_bestparentif; /* interface for best parent */

	/* flag that it is time to send DIO */
	bool               mTimeToSendDio;

	/* flag that it is time to send DAO */
	bool               mTimeToSendDao;
	char               mDagName[64];

	/* RiPpLe statistics */
	enum RPL_DIO_MOP        mMode;
	unsigned short          mDTSN;
	unsigned short 			mDAOSequence;
	unsigned int            mInstanceid;
	unsigned int            mLifetime;      /* dag lifetime */
	unsigned int            mVersion;
	unsigned int            mDio_lifetime;  /* dio lifetime */
	bool                    mGrounded;
	unsigned int            mInterval_msec;

	/* must be class, due to forward reference */
	class rpl_event        *mSendDioEvent;  /* when to send a DIO */
	class rpl_event        *mSendDaoEvent;  /* when to send a DAO */

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

