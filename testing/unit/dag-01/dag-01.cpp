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

/* TEST1: a DN shall have a dagid */
static void t1(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='1';
        
        class dag_network dn(d);

        assert(dn.mDagid[0]=='T');
        assert(dn.mDagid[1]=='1');
}

/* TEST2: a DN can be found by a dagid */
static void t2(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='2';
        
        /* make a new dn */
        class dag_network *dn1 = new dag_network(d);
        class dag_network *dn2 = dag_network::find_by_dagid(d);

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
        class dag_network *dn1 = new dag_network(d);

        /* now look for a phony dag network */
        d[0]='N';
        class dag_network *dn2 = dag_network::find_by_dagid(d);
        assert(dn2 == NULL);
}

/* TEST4: remove from list, twice to make sure */
static void t4(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='4';
        
        /* make a new dn */
        class dag_network *dn1 = new dag_network(d);

        dn1->remove_from_list();

        /* verify that we can not find it */
        class dag_network *dn2 = dag_network::find_by_dagid(d);
        assert(dn2 == NULL);

        dn1->remove_from_list();

        /* verify that we can not find it */
        class dag_network *dn3 = dag_network::find_by_dagid(d);
        assert(dn3 == NULL);
}

/* TEST5: look for a new dagid */
static void t5(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='5';
        
        /* verify that we can not find it */
        class dag_network *dn2 = dag_network::find_or_make_by_dagid(d, true, stderr);
        assert(dn2 != NULL);

        assert(dn2->mDagid[0]=='T');
        assert(dn2->mDagid[1]=='5');
}

int main(int argc, char *argv[])
{
	int i;

        printf("dag-01 t1\n");        t1();
        printf("dag-01 t2\n");        t2();
        printf("dag-01 t3\n");        t3();
        printf("dag-01 t4\n");        t4();
        printf("dag-01 t5\n");        t5();
	exit(0);
}


