/*
 * Unit tests for processing a DIO.
 *
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <assert.h>
}

#include "iface.h"
#include "dag.h"
#include "node.h"

/* TEST1: instantiate an RPL_node */
static void t1(rpl_debug *deb, class dag_network *dn1)
{
    class rpl_node *n = new rpl_node;
    delete n;
}

/* TEST2: instantiate an RPL node from an IPv6 string name  */
static void t2(rpl_debug *deb, class dag_network *dn1)
{
    class rpl_node *valid1   = new rpl_node("2001:db8::abcd:0123", dn1, deb);
    class rpl_node *invalid1 = new rpl_node("2001:db8::/32", dn1, deb);
    class rpl_node *invalid2 = new rpl_node("2001:db8:abcd:0123", dn1, deb);

    assert(valid1->validP());
    assert(!invalid1->validP());
    assert(!invalid2->validP());

    delete valid1;
    delete invalid1;
    delete invalid2;
}

/* TEST3: instantiate an RPL node from an IPv6 string name  */
static void t3(rpl_debug *deb, class dag_network *dn1)
{
    class rpl_node *n1    = new rpl_node("2001:db8::abcd:0123", dn1, deb);
    time_t nn = 1000;

    /* nodes have names */
    n1->set_name("frank");
    assert(strcmp(n1->node_name(), "frank")==0);

    /* nodes record last seen value */
    n1->set_last_seen(nn);
    assert(n1->get_last_seen() == nn);

    delete n1;
}

/* TEST4: instantiate an RPL node_map and insert
 *        some nodes into it.
 */
static void t4(rpl_debug *deb, class dag_network *dn1)
{
    class rpl_node n1("2001:db8::abcd:0001", dn1, deb);
    class rpl_node n2("2001:db8::abcd:0002", dn1, deb);
    class rpl_node n3("2001:db8::abcd:0003", dn1, deb);
    class rpl_node n4("2001:db8::abcd:0001", dn1, deb);  // duplicate

    n1.set_name("n1");
    n2.set_name("n2");
    n3.set_name("n3");
    n4.set_name("n4");

    /* create a new empty set */
    node_map ns;

    assert(ns.size() == 0);

    assert(ns.insert(node_pair(n1.node_number(), n1)).second == true);
    assert(ns.size() == 1);

    assert(ns.insert(node_pair(n2.node_number(), n2)).second == true);
    assert(ns.size() == 2);

    assert(ns.insert(node_pair(n3.node_number(), n3)).second == true);
    assert(ns.size() == 3);

    assert(ns.insert(node_pair(n4.node_number(), n4)).second == false);
    assert(ns.size() == 3);
}

/* TEST5: instantiate an RPL node_map and insert
 *        some nodes into it, using in_addr6 directly.
 */
static void t5(rpl_debug *deb, class dag_network *dn1)
{
    struct in6_addr a1;
    inet_pton(AF_INET6, "2001:db8::abcd:0001", &a1);
    class rpl_node n1(a1);
    class rpl_node n4(a1);

    struct in6_addr a2;
    inet_pton(AF_INET6, "2001:db8::abcd:0002", &a2);
    class rpl_node n2(a2);

    struct in6_addr a3;
    inet_pton(AF_INET6, "2001:db8::abcd:0003", &a3);
    class rpl_node n3(a3);

    struct in6_addr a5;
    inet_pton(AF_INET6, "2001:db8::abcd:0005", &a5);

    n1.set_name("n1");
    n2.set_name("n2");
    n3.set_name("n3");
    n3.set_name("n4");

    /* create a new empty set */
    node_map ns;

    assert(ns.size() == 0);

    assert(ns.insert(node_pair(n1.node_number(), n1)).second == true);
    assert(ns.size() == 1);

    assert(ns.insert(node_pair(n2.node_number(), n2)).second == true);
    assert(ns.size() == 2);

    assert(ns.insert(node_pair(n3.node_number(), n3)).second == true);
    assert(ns.size() == 3);

    assert(ns.insert(node_pair(n4.node_number(), n4)).second == false);
    assert(ns.size() == 3);

    /* now look for them again */
    node_map_iterator ni = ns.find(a1);
    rpl_node   *rn =  &ni->second;
    const char *nn =  rn->node_name();
    assert(strcmp(nn, "n1")==0);

    /* now look up an invalid item! */
    node_map_iterator ni2= ns.find(a5);
    assert(ni2 == ns.end());
}

int main(int argc, char *argv[])
{
	int i;

        rpl_debug *deb = new rpl_debug(true, stdout);
        class dag_network *dn1 = new dag_network("07-dagset");

        printf("dagset-01 t1\n");     t1(deb, dn1);
        printf("dagset-01 t2\n");     t2(deb, dn1);
        printf("dagset-01 t3\n");     t3(deb, dn1);
        printf("dagset-01 t4\n");     t4(deb, dn1);
        printf("dagset-01 t5\n");     t5(deb, dn1);

        printf("dagset-01 tests finished\n");
	exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

