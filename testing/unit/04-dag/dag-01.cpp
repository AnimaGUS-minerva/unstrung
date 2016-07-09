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
#include "debug.h"

rpl_debug *debug;

/* TEST1: a DN shall have a dagid, which is an IPv6 address */
static void t1(void)
{
    struct in6_addr t1name;
    inet_pton(AF_INET6, "2001:DB8:0001:0002::babe", &t1name);

    class dag_network dn(1, &t1name, debug);
    assert(dn.mDagid[0]==0x20);
    assert(dn.mDagid[1]==0x01);
}

/* TEST1b: a DN shall have a dagid, which is an IPv6 address */
static void t1b(void)
{
    class dag_network dn(1, "2001:DB8:0001:0002::babe", debug);
    assert(dn.mDagid[0]==0x20);
    assert(dn.mDagid[1]==0x01);
}

/* TEST2: a DN can be found by a dagid */
static void t2(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='2';

        /* make a new dn */
        class dag_network *dn1 = new dag_network(1, d, debug);
        class dag_network *dn2 = dag_network::find_by_instanceid(1, d);

        assert(dn1 == dn2);
}

/* TEST3: a missing DN should never be found */
static void t3(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='3';

        /* make a new dn */
        class dag_network *dn1 = new dag_network(1, d, debug);
        class dag_network *dn1b = dag_network::find_by_instanceid(1, d);
        assert(dn1b != NULL);

        /* now look for a phony instance ID network */
        class dag_network *dn2 = dag_network::find_by_instanceid(2, d);
        assert(dn2 == NULL);

        /* now look for a network with different dagID: should be found */
        d[0]='N';
        class dag_network *dn3 = dag_network::find_by_instanceid(1, d);
        assert(dn3 != NULL);
}

/* TEST4: remove from list, twice to make sure */
static void t4(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='4';

        /* make a new dn */
        class dag_network *dn1 = new dag_network(99, d, debug);

        dn1->remove_from_list();

        /* verify that we can not find it */
        class dag_network *dn2 = dag_network::find_by_instanceid(99, d);
        assert(dn2 == NULL);

        dn1->remove_from_list();

        /* verify that we can not find it */
        class dag_network *dn3 = dag_network::find_by_instanceid(99, d);
        assert(dn3 == NULL);
}

/* TEST5: look for a new instanceID */
static void t5(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='5';

        /* verify that we can not find it */
        class dag_network *dn2 = dag_network::find_or_make_by_instanceid(99, d, debug, false);
        assert(dn2 != NULL);

        assert(dn2->mDagid[0]=='T');
        assert(dn2->mDagid[1]=='5');
}

int main(int argc, char *argv[])
{
	int i;

        debug = new rpl_debug(true, stderr);

        printf("dag-01 t1\n");        t1(); t1b();
        printf("dag-01 t2\n");        t2();
        printf("dag-01 t3\n");        t3();
        printf("dag-01 t4\n");        t4();
        printf("dag-01 t5\n");        t5();
	exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make check"
 * End:
 */


