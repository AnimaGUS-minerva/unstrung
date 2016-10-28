#ifndef _UNSTRUNG_NODE_H_
#define _UNSTRUNG_NODE_H_

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <string.h>
}

#include <map>
#include "oswlibs.h"
#include "debug.h"
#include "grasp.h"

class dag_network;
class network_interface;
class rpl_less;
class na_construction;

#define EUI64_BUF_LEN 20       /* space for 4x 4hex digit, plus 3 colons, plus \0 */


class rpl_node {
        friend class rpl_less;
public:
        rpl_node() { valid = false; name[0]='\0'; self = false; };
        rpl_node(const char *ipv6,
                 dag_network *dn, rpl_debug *deb);
        rpl_node(const struct in6_addr v6);
        rpl_node(const struct in6_addr v6,
                 dag_network *dn, rpl_debug *deb);
        bool validP() { return valid; };

        void   set_last_seen(time_t clock) { lastseen = clock; };
        time_t get_last_seen() { return lastseen; };

        void set_addr(const char *ipv6);
        void set_addr(const struct in6_addr v6);
        void set_name(const char *nodename) {
            name[0]='\0';
            strncat(name, nodename, sizeof(name));
        };
        const char *node_name(void);
        struct in6_addr& node_number() { return nodeip.u.v6.sin6_addr; };
        ip_address &node_address() { return nodeip; };
        void  makevalid(const struct in6_addr v6,
                        dag_network *dn, rpl_debug *deb);
        void  set_dag(dag_network *dn, rpl_debug *deb);
        rpl_debug *debug;
        void  markself(int index) {
            this->self = true;
            ifindex    = index;
            calc_name();
        };
        void  markvalid(int index, struct in6_addr v6, rpl_debug *deb) {
            nodeip.u.v6.sin6_addr = v6;
            nodeip.u.v6.sin6_family=AF_INET6;
            ifindex = index;
            valid   = true;
            calc_name();
            if(deb != NULL) {
                debug = deb;
            } else {
                debug = NULL;
            }
        };
        bool  isself() { return self; };
        unsigned int get_ifindex() { return ifindex; };
        bool  is_equal(const struct in6_addr v6) {
            if(!valid) return false;
            return (memcmp(nodeip.u.v6.sin6_addr.s6_addr, v6.s6_addr, 16)==0);
        };

        void update_nce_stamp(void);
        int  build_basic_neighbour_advert(network_interface *iface,
                                          bool solicited,
                                          unsigned char *buff, unsigned int buff_len);

        void reply_mcast_neighbour_advert(network_interface *iface,
                                          struct in6_addr from,
                                          struct in6_addr ip6_to,
                                          const  time_t now,
                                          struct nd_neighbor_solicit *ns,
                                          const int nd_len);

        void reply_mcast_neighbour_join(network_interface *iface,
                                        struct in6_addr from,
                                        struct in6_addr ip6_to,
                                        const  time_t now,
                                        struct nd_neighbor_solicit *ns,
                                        const int nd_len);

        void reply_neighbour_advert(network_interface *iface,
                                    unsigned int success);

        void add_route_via_node(ip_subnet &prefix, network_interface *iface);

        bool join_declined(void) { return alreadyDeclined; };
        bool join_accepted(void) { return alreadyAccepted; };
        void set_accepted(bool accepted) {
            alreadyDeclined = !accepted;
            alreadyAccepted = accepted;
            joinQueryStarted = false;
        };

        bool join_queryInProgress(void) { return joinQueryStarted; };
        void start_joinQuery(void);

        static rpl_node *find_by_addr(struct in6_addr v6);

        /* probably should be made accessible via friend function */
        grasp_session_id  queryId;

        static char *fmt_eui64(const unsigned char eui64[8], char *buf, size_t buf_len);

protected:
	ip_address        nodeip;
        unsigned int      ethertype;      /* ARPHRD_ETHER, or ARPHRD_IEEE802154 */
        unsigned char     l2addr[8];

        void start_neighbour_advert(na_construction &progress);
        int  end_neighbour_advert(na_construction &progress);

        bool              alreadyDeclined;
        bool              alreadyAccepted;
        bool              joinQueryStarted;

private:
        bool       valid;
        bool       self;
        int        ifindex;
        time_t     lastseen;
        char       name[INET6_ADDRSTRLEN+10];
        dag_network *mDN;          /* should be shared_ptr */
        void       couldBeValid(void);
        void       calc_name(void);
};

class rpl_less {
public:
    bool operator()(const struct in6_addr &x, const struct in6_addr &y) const {
        return memcmp(x.s6_addr, y.s6_addr, 16) < 0;
    }
};

typedef std::map<struct in6_addr, rpl_node , rpl_less>           node_map;
typedef std::map<struct in6_addr, rpl_node , rpl_less>::iterator node_map_iterator;
typedef std::pair<struct in6_addr, rpl_node> node_pair;

#endif /* NODE_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

