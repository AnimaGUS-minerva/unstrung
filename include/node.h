#ifndef _UNSTRUNG_NODE_H_
#define _UNSTRUNG_NODE_H_

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <string.h>
}

#include <set>

class rpl_less;
class rpl_node {
        friend class rpl_less;
public:
        rpl_node() { valid = true; };
        rpl_node(const char *ipv6);
        bool validP() { return valid; };

        void   set_last_seen(time_t clock) { lastseen = clock; };
        time_t get_last_seen() { return lastseen; };

        void set_name(const char *nodename) {
            name[0]='\0';
            strncat(name, nodename, sizeof(name));
        };
        const char *node_name() { return name; };

protected:
        struct in6_addr nodeip;

private:
        bool       valid;
        time_t     lastseen;
        char       name[16];
};

class rpl_less {
public:
        bool operator()(const rpl_node &x, const rpl_node &y) const {
                return memcmp(x.nodeip.s6_addr, y.nodeip.s6_addr, 16) < 0;
        }
};

typedef std::set<rpl_node, rpl_less> node_set;

#endif /* NODE_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

