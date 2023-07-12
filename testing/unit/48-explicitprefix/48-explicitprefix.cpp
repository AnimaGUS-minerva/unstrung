#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

int main(int argc, char *argv[])
{
        pcap_network_interface *iface = NULL;
        pcap_network_interface *iface2 = NULL;
        struct in6_addr iface_src2;

        rpl_debug *deb = new rpl_debug(true, stdout);
        inet_pton(AF_INET6, "fe80::1000:ff:fe64:6423", &iface_src2);

        /* now finish setting things up with netlink */
        pcap_network_interface::scan_devices(deb, false);

        // set up for sending a DIO packet, and actually do it.
        //  --pcapout ./OUTPUTS/senddio-test-03.pcap --fake -i wlan0 -v
        //  --instanceid 42 --dagid 2001:db8:661e::1 --prefix 2001:db8:0001::/48
        //  --prefixlifetime 12 --grounded --storing --version 1
        //  --sequence 10 --rank 2

        dag_network *dn = new dag_network(42, "2001:db8:661e::1", deb);
        ip_subnet prefix;
        err_t e = ttosubnet("2001:db8:0001::/48", 0, AF_INET6, &prefix);
	dn->set_active();
	dn->set_debug(deb);
        dn->set_prefix(prefix);
        dn->set_prefixlifetime(12);
        dn->set_grounded(true);
        dn->set_mode(RPL_DIO_STORING_MULTICAST);
        dn->set_version(1);
        dn->set_sequence(10);
        dn->set_dagrank(2);

        iface = pcap_network_interface::setup_infile_outfile("wlan0",
                                                             "../INPUTS/daoE.pcap",
                                                             "../OUTPUTS/17-node-A-out.pcap",
                                                             deb);
        dn->addselfprefix(iface);

	exit(0);
}


