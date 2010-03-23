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
#include <time.h>
}

#include "iface.h"
#include "dag.h"

struct in6_addr dummy_src1;
time_t now;

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
        d1.rpl_dagid[0]='T';
        d1.rpl_dagid[0]='1';

        d1.rpl_seq = 1;
        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 1);
        assert(dn->mStats[PS_SEQ_OLD] == 0);

        d1.rpl_seq = 4;
        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 4);
        assert(dn->mStats[PS_SEQ_OLD] == 0);

        d1.rpl_seq = 3;
        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 4);
        assert(dn->mStats[PS_SEQ_OLD] == 1);

        d1.rpl_seq = 240;
        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 4);
        assert(dn->mStats[PS_SEQ_OLD] == 2);

        d1.rpl_seq = 130;
        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 130);
        assert(dn->mStats[PS_SEQ_OLD] == 2);

        d1.rpl_seq = 243;
        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 243);
        assert(dn->mStats[PS_SEQ_OLD] == 2);

        d1.rpl_seq = 1;
        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 1);
        assert(dn->mStats[PS_SEQ_OLD] == 2);

        d1.rpl_seq = 1;
        unsigned int processed_count = dn->mStats[PS_PACKET_PROCESSED];
        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 1);
        assert(dn->mStats[PS_PACKET_PROCESSED] == processed_count+1);
}

/*
 * in this test case, the system should note that it sees a dagrank
 * lower than what it currently has (which is infinite).
 *
 */
static void t3(void)
{
        struct nd_rpl_dio d1;

        /* make sure dag rank starts as infinite */
        assert(dn->dag_rank_infinite());

        memset(&d1, 0, sizeof(d1));

        d1.rpl_seq = 2;
        d1.rpl_dagid[0]='T';
        d1.rpl_dagid[0]='1';
        d1.rpl_instanceid = 1;
        d1.rpl_dagrank = 1;

        unsigned int lower_rank_count = dn->mStats[PS_LOWER_RANK_CONSIDERED];

        dn->receive_dio(dummy_src1, now, &d1, sizeof(d1));
        assert(dn->mStats[PS_LOWER_RANK_CONSIDERED] == lower_rank_count+1);
}
        

int main(int argc, char *argv[])
{
	int i;

        struct in6_addr dummy_src1;
        inet_pton(AF_INET6, "2001:db8::abcd:00a1", &dummy_src1);
        time_t now;
        time(&now);

        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='1';
        
        dn = new dag_network(d);

        printf("dag-02 t1\n");        t1();
        printf("dag-02 t2\n");        t2();
        delete dn;

        dn = new dag_network(d);
        printf("dag-02 t3\n");        t3();
	exit(0);
}


