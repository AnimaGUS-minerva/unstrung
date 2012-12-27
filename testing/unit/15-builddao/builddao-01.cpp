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
#include "dao.h"

#include "fakeiface.h"

/* TEST1: send out a basic DAO */
/*
 *   To do this we need to have a dag created with a prefix,
 *   and this has to be attached to an interface.
 *
 *   We create the body that ICMPv6 should send out.
 */
static void t1(rpl_debug *deb)
{
    pcap_network_interface::scan_devices(deb);

    pcap_network_interface *my_if = (pcap_network_interface *)network_interface::find_by_name("wlan0");

    assert(my_if!=NULL);

    dagid_t dag1name;
    memset(dag1name, 0, DAGID_LEN);
    dag1name[0]='T';
    dag1name[1]='1';

    class dag_network dag1(dag1name);

    my_if->set_pcap_out("../OUTPUTS/15-dao.pcap", DLT_EN10MB);
    my_if->set_rpl_dagid(dag1name);

    unsigned char buf[2048];
    ip_subnet out;
    ttosubnet("2001:db8::abcd:0002/128",0, AF_INET6, &out);

    /* make a new dn */
    class dag_network *dn1 = new dag_network(dag1name);

    /* add a host to this network */
    rpl_node n1("2001:db8::abcd:0002/128");
    struct in6_addr via1;
    inet_pton(AF_INET6, "2001:db8::abcd:1005", &via1);
    prefix_node pvia1(deb, via1, 128);
    dn1->addprefix(n1, my_if, pvia1.get_prefix());

    int len = my_if->build_dao(buf, 2048, dn1);
    assert(len == 56);

    my_if->send_raw_icmp(NULL, buf, len);
}

int main(int argc, char *argv[])
{
    int i;

    rpl_debug *deb = new rpl_debug(true, stderr);
    printf("builddao-01 t1\n");     t1(deb);

    printf("builddao-01 tests finished\n");
    exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

