#ifndef _UNSTRUNG_NODE_H_
#define _UNSTRUNG_NODE_H_

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
}

#include <set>

class rpl_less;
class rpl_node {
        friend class rpl_less;
public:
protected:
        struct in6_addr nodeip;
private:
};

class rpl_less {
public:
        bool operator()(rpl_node &x, rpl_node &y) const {
                return memcmp(x.nodeip.s6_addr, y.nodeip.s6_addr, 16) < 0;
        }
};

class node_set : std::set<rpl_node, rpl_less> {
};

#endif /* NODE_H */
