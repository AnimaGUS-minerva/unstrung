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
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <assert.h>
}

#include "iface.h"
#include "dag.h"
#include "node.h"

/* TEST1: instantiate an RPL_node */
static void t1(void)
{
    class rpl_node *n = new rpl_node;
    delete n;
}

/* TEST2: instantiate an RPL node from an IPv6 string name  */
static void t2(void)
{
    class rpl_node *valid1   = new rpl_node("2001:db8::abcd:0123");
    class rpl_node *invalid1 = new rpl_node("2001:db8::/32");
    class rpl_node *invalid2 = new rpl_node("2001:db8:abcd:0123");

    assert(valid1->validP());
    assert(!invalid1->validP());
    assert(!invalid2->validP());

    delete valid1;
    delete invalid1;
    delete invalid2;
}

/* TEST3: instantiate an RPL node from an IPv6 string name  */
static void t3(void)
{
    class rpl_node *n1    = new rpl_node("2001:db8::abcd:0123");
    time_t nn = 1000;

    /* nodes have names */
    n1->set_name("frank");
    assert(strcmp(n1->node_name(), "frank")==0);

    /* nodes record last seen value */
    n1->set_last_seen(nn);
    assert(n1->get_last_seen() == nn);
    
    delete n1;
}

/* TEST4: instantiate an RPL node_set and insert
 *        some nodes into it.
 */
static void t4(void)
{
    class rpl_node n1("2001:db8::abcd:0001");
    class rpl_node n2("2001:db8::abcd:0002");
    class rpl_node n3("2001:db8::abcd:0003");
    class rpl_node n4("2001:db8::abcd:0001");  // duplicate

    n1.set_name("n1");
    n2.set_name("n2");
    n3.set_name("n3");
    n4.set_name("n4");
    
    /* create a new empty set */
    node_set ns;

    assert(ns.size() == 0);
    
    assert(ns.insert(n1).second == true);
    assert(ns.size() == 1);

    assert(ns.insert(n2).second == true);
    assert(ns.size() == 2);

    assert(ns.insert(n3).second == true);
    assert(ns.size() == 3);

    assert(ns.insert(n4).second == false);
    assert(ns.size() == 3);
}

int main(int argc, char *argv[])
{
	int i;

        printf("dagset-01 t1\n");     t1();
        printf("dagset-01 t2\n");     t2();
        printf("dagset-01 t3\n");     t3();
        printf("dagset-01 t4\n");     t4();

        printf("dagset-01 tests finished\n");
	exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

