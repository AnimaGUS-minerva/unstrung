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
#include "debug.h"

class dag_network;
class rpl_less;
class rpl_node {
        friend class rpl_less;
public:
        rpl_node() { valid = false; name[0]='\0'; };
        rpl_node(const char *ipv6);
        rpl_node(const struct in6_addr v6);
        bool validP() { return valid; };

        void   set_last_seen(time_t clock) { lastseen = clock; };
        time_t get_last_seen() { return lastseen; };

        void set_name(const char *nodename) {
            name[0]='\0';
            strncat(name, nodename, sizeof(name));
        };
        const char *node_name();
        const struct in6_addr& node_number() { return nodeip; };
        void  makevalid(const struct in6_addr v6,
                        const dag_network *dn, rpl_debug *deb);
        rpl_debug *debug;
        void  markself(int index) {
            this->self = true;
            ifindex    = index;
        };
        
protected:
        struct in6_addr nodeip;

private:
        bool       valid;
        bool       self;
        int        ifindex;
        time_t     lastseen;
        char       name[INET6_ADDRSTRLEN];
        const dag_network *mDN;          /* should be shared_ptr */
};

class rpl_less {
public:
    bool operator()(const struct in6_addr &x, const struct in6_addr &y) const {
        return memcmp(x.s6_addr, y.s6_addr, 16) < 0;
    }
};

typedef std::map<struct in6_addr, rpl_node, rpl_less>           node_map;
typedef std::map<struct in6_addr, rpl_node, rpl_less>::iterator node_map_iterator;
typedef std::pair<struct in6_addr, rpl_node> node_pair;

#endif /* NODE_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

