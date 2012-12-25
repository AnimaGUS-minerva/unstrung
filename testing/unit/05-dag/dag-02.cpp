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
#include "fakeiface.h"
#include "dag.h"

struct in6_addr dummy_src1;
struct in6_addr iface_src2;
time_t now;

class dag_network *dn = NULL;

/* TEST1: a DN shall have a sequence number */
static void t1(network_interface *iface)
{
        assert(dn->last_seq() == 0);
}

/* TEST2: a DN will update it's sequence number, if
 *        the sequence number is not too old
 */
static void t2(network_interface *iface)
{
        struct nd_rpl_dio d1;

        memset(&d1, 0, sizeof(d1));
        d1.rpl_dagid[0]='T';
        d1.rpl_dagid[0]='1';

        d1.rpl_dtsn = 1;
        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 1);
        assert(dn->mStats[PS_SEQ_OLD] == 0);

        d1.rpl_dtsn = 4;
        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 4);
        assert(dn->mStats[PS_SEQ_OLD] == 0);

        d1.rpl_dtsn = 3;
        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 4);
        assert(dn->mStats[PS_SEQ_OLD] == 1);

        d1.rpl_dtsn = 240;
        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 4);
        assert(dn->mStats[PS_SEQ_OLD] == 2);

        d1.rpl_dtsn = 130;
        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 130);
        assert(dn->mStats[PS_SEQ_OLD] == 2);

        d1.rpl_dtsn = 243;
        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 243);
        assert(dn->mStats[PS_SEQ_OLD] == 2);

        d1.rpl_dtsn = 1;
        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 1);
        assert(dn->mStats[PS_SEQ_OLD] == 2);

        d1.rpl_dtsn = 1;
        unsigned int processed_count = dn->mStats[PS_PACKET_PROCESSED];
        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->last_seq() == 1);
        assert(dn->mStats[PS_PACKET_PROCESSED] == processed_count+1);
}

/*
 * in this test case, the system should note that it sees a dagrank
 * lower than what it currently has (which is infinite).
 *
 */
static void t3(network_interface *iface)
{
        struct nd_rpl_dio d1;

        /* make sure dag rank starts as infinite */
        assert(dn->dag_rank_infinite());

        memset(&d1, 0, sizeof(d1));

        d1.rpl_dtsn = 2;
        d1.rpl_dagid[0]='T';
        d1.rpl_dagid[0]='1';
        d1.rpl_instanceid = 1;
        d1.rpl_dagrank = 1;

        unsigned int lower_rank_count = dn->mStats[PS_LOWER_RANK_CONSIDERED];

        dn->receive_dio(iface, dummy_src1, now, &d1, sizeof(d1));
        assert(dn->mStats[PS_LOWER_RANK_CONSIDERED] == lower_rank_count+1);
}

/*
 * in this test case, the system should create a new node in the dag_member.
 * 
 */
static void t4(network_interface *iface)
{
        struct nd_rpl_dio d1;

        struct in6_addr a1;
        inet_pton(AF_INET6, "2001:db8::abcd:0001", &a1);

        /* make sure dag rank starts as infinite */
        assert(dn->dag_rank_infinite());

        memset(&d1, 0, sizeof(d1));

        d1.rpl_dtsn = 2;
        d1.rpl_dagid[0]='T';
        d1.rpl_dagid[0]='1';
        d1.rpl_instanceid = 1;
        d1.rpl_dagrank = 1;

        dn->receive_dio(iface, a1, now, &d1, sizeof(d1));

        assert(dn->member_count() == 1);
        assert(dn->contains_member(a1));
}
        

int main(int argc, char *argv[])
{
	int i;

        inet_pton(AF_INET6, "2001:db8::abcd:00a2", &iface_src2);
        inet_pton(AF_INET6, "2001:db8::abcd:00a1", &dummy_src1);

        time_t now;
        time(&now);

        int pcap_link = DLT_EN10MB;
	pcap_t *pout = pcap_open_dead(pcap_link, 65535);
	if(!pout) {
		fprintf(stderr, "can not create pcap_open_deads\n");
		exit(1);
	}
		
	pcap_dumper_t *out = pcap_dump_open(pout, "t1.pcap");
        pcap_network_interface *iface = new pcap_network_interface(out);

        rpl_debug *deb = new rpl_debug(false, stdout);
        iface->set_debug(deb);
        iface->set_if_addr(iface_src2);
	
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='1';
        
        dn = new dag_network(d);
        dn->set_debug(deb);
	dn->set_active();

        printf("dag-02 t1\n");        t1(iface);
        printf("dag-02 t2\n");        t2(iface);
        delete dn;

        dn = new dag_network(d);
        printf("dag-02 t3\n");        t3(iface);
        delete dn;

        dn = new dag_network(d);
        printf("dag-02 t4\n");        t4(iface);
        delete dn;

	exit(0);
}


