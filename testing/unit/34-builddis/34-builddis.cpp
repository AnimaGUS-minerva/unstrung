/*
 * Unit tests for creating a DAO Information Solicitation.
 * This is usually multicast.
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

/* TEST1: send out a DIS */
/*
 *   To do this we need to have an interface on which to operate, and a instanceID
 *   which we are interested in.
 *   That is all.
 */
int main(int argc, char *argv[])
{
    int i;
    pcap_network_interface *iface = NULL;

    rpl_debug *deb = new rpl_debug(true, stdout);

    /* now finish setting things up with netlink */
    pcap_network_interface::scan_devices(deb, false);


    iface = pcap_network_interface::setup_infile_outfile("wlan0",
                                                         NULL,
                                                         "../OUTPUTS/34-node-E-dis.pcap",
                                                         deb);
    /* setup of src for packets to be node E */
    struct in6_addr iface_src2;
    inet_pton(AF_INET6, "fe80::1000:ff:fe64:6602", &iface_src2);

    iface->set_debug(deb);
    iface->set_if_index(1);
    iface->set_if_addr(iface_src2);

    class dag_network dn(2, &iface_src2, deb);

    /* build a DIS, send it. */
    iface->send_dis(&dn);

    printf("34-build_dis tests finished\n");
    exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * compile-command: "make check"
 * End:
 */

