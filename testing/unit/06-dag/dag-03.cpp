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
#include "node.h"
#include "dio.h"
#include "fakeiface.h"

struct in6_addr dummy_src1;
time_t now;

class dag_network *dn = NULL;

/*
 * TEST1: a DN will get a new prefix, if it is new.
 */
static void t1(rpl_debug *deb)
{
        class rpl_node n1("2001:db8::abcd:0002");

        int pcap_link = DLT_EN10MB;
	pcap_t *pout = pcap_open_dead(pcap_link, 65535);
	if(!pout) {
		fprintf(stderr, "can not create pcap_open_deads\n");
		exit(1);
	}
		
	pcap_dumper_t *out = pcap_dump_open(pout, "t1.pcap");
        pcap_network_interface *iface = new pcap_network_interface(out);

        iface->set_debug(deb);

        /* test data from senddio-test-04.out */
        u_int8_t diodata[]={
                /*ICMPv6 header 0x9b, 0x02, 0x00, 0x00,*/
                0x00, 0x0a, 0x2a, 0x01,
                0x74, 0x68, 0x69, 0x73, 0x69, 0x73, 0x6d, 0x79,  
                0x6e, 0x69, 0x63, 0x65, 0x64, 0x61, 0x67, 0x31,
                0x03, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 
                0x30, 0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0x00  
        };
        int dio_data_len=sizeof(diodata);

        

        rpl_dio dio(n1, (struct nd_rpl_dio *)diodata,
                    dio_data_len);

        ip_subnet prefix;
        const char *example="2001:db8:0001::/48";
        ttosubnet(example, strlen(example), AF_INET6, &prefix);

        dn->addprefix(n1, iface, prefix);

        assert(dn->prefixcount() >= 1);
}


int main(int argc, char *argv[])
{
	int i;

        rpl_debug *deb = new rpl_debug(false, stdout);

        struct in6_addr dummy_src1;
        inet_pton(AF_INET6, "2001:db8::abcd:00a1", &dummy_src1);
        time_t now;
        time(&now);

        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='3';
        
        dn = new dag_network(d);
        dn->set_debug(deb);

        printf("dag-03 t1\n");        t1(deb);
        delete dn;

	exit(0);
}


