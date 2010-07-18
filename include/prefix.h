#ifndef _UNSTRUNG_PREFIX_H_
#define _UNSTRUNG_PREFIX_H_

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <string.h>
#include "oswlibs.h"
}

#include <map>

class network_interface;
class dag_network;
class rpl_node;
class prefix_less;
class prefix_node {
        friend class prefix_less;
public:
        prefix_node() {
            valid = false;
        };
        prefix_node(rpl_node *announcer, ip_subnet sub);
        prefix_node(const struct in6_addr v6, const int prefixlen);
        bool validP() { return valid; };

        const char *node_name();
        const ip_subnet &prefix_number() { return mPrefix; };
        void configureip(void);
        void set_announcer(rpl_node *announcer) {
            announced_from = announcer;
        };
        void set_dn(dag_network *dn) {
            mDN = dn;
        };
        void set_prefix(const struct in6_addr v6, const int prefixlen);
        void set_prefix(ip_subnet prefix);
        
protected:
        ip_subnet    mPrefix;
        void         verbose_log(const char *fmt, ...);

private:
        bool         valid;
        rpl_node    *announced_from;       /* should be shared_ptr */
        char         name[SUBNETTOT_BUF];
        dag_network *mDN;          /* should be shared_ptr */
};

class prefix_less {
public:
    bool operator()(const ip_subnet &x, const ip_subnet &y) const {
        /* sort by IP address first */
        int match=memcmp(x.addr.u.v6.sin6_addr.s6_addr,
                         y.addr.u.v6.sin6_addr.s6_addr, 16);
        if(match < 0 || match > 0) {
            return match;
        }
        /* must be identical IP addresses */
        match = x.maskbits - y.maskbits;
        return match;
    }
};

typedef std::map<ip_subnet, prefix_node, prefix_less>           prefix_map;
typedef std::map<ip_subnet, prefix_node, prefix_less>::iterator prefix_map_iterator;
typedef std::pair<ip_subnet, prefix_node> prefix_pair;

#endif /* PREFIX_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

