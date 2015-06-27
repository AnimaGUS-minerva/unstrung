/*
 * Unit tests for processing a DAOACK.
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

/* TEST1: send out a DAOACK */
/*
 *   To do this we need to have a dag created with a prefix,
 *   and this has to be attached to an interface.
 *
 *   We create the body that ICMPv6 should send out.
 */
static void t1(rpl_debug *deb)
{
    pcap_network_interface::scan_devices(deb, false);

    pcap_network_interface *my_if = (pcap_network_interface *)network_interface::find_by_name("wlan0");

    assert(my_if!=NULL);

    dagid_t dag1name;
    memset(dag1name, 0, DAGID_LEN);
    dag1name[0]='T';
    dag1name[1]='1';

    /* create self, DODAG */
    my_if->set_pcap_out("../OUTPUTS/26-daoack.pcap", DLT_EN10MB);

    unsigned char buf[2048];
    ip_subnet out;
    /* this is who *I* am, this is what will be announced in the DAOACK */
    ttosubnet("2001:db8::abcd:0001/128",0, AF_INET6, &out);

    /* make a new dn */
    class dag_network *dn1 = new dag_network(dag1name, deb);
    dn1->set_active();

    /* who sent us a DAO */
    rpl_node announced_from("fe80::216:3eff:fe22:4455", dn1, deb);
    assert(announced_from.validP());

    /* build a DAOACK, send it. */
    my_if->send_daoack(announced_from, *dn1, 5);
}

int main(int argc, char *argv[])
{
    int i;

    rpl_debug *deb = new rpl_debug(true, stdout);
    printf("26-builddaoack t1\n");     t1(deb);

    printf("26-builddaoack tests finished\n");
    exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make check"
 * End:
 */

