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

class dag_network *dn = NULL;

/* TEST1: a DN shall have a sequence number */
static void t1(void)
{
        assert(dn->last_seq() == 0);
}

/* TEST2: a DN will update it's sequence number, if
 *        the sequence number is not too old
 */
static void t2(void)
{
        struct nd_rpl_dio d1;

        memset(&d1, 0, sizeof(d1));

        d1.rpl_seq = 1;
        dn->receive_dio(&d1, sizeof(d1));
        assert(dn->last_seq() == 1);
        assert(dn->mDiscards[DR_SEQOLD] == 0);

        d1.rpl_seq = 4;
        dn->receive_dio(&d1, sizeof(d1));
        assert(dn->last_seq() == 4);
        assert(dn->mDiscards[DR_SEQOLD] == 0);

        d1.rpl_seq = 3;
        dn->receive_dio(&d1, sizeof(d1));
        assert(dn->last_seq() == 4);
        assert(dn->mDiscards[DR_SEQOLD] == 1);
}

int main(int argc, char *argv[])
{
	int i;

        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='1';
        
        dn = new dag_network(d);

        printf("dag-02 t1\n");        t1();
        printf("dag-02 t2\n");        t2();
	exit(0);
}


