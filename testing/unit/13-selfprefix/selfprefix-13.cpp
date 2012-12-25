#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

int main(int argc, char *argv[])
{
        pcap_network_interface *iface = NULL;
        pcap_network_interface *iface2 = NULL;
        struct in6_addr iface_src2;


        rpl_debug *deb = new rpl_debug(true, stderr);
        inet_pton(AF_INET6, "fe80::1000:ff:fe64:6423", &iface_src2);

        /* now finish setting things up with netlink */
        pcap_network_interface::scan_devices(deb);

        iface = pcap_network_interface::setup_infile_outfile("wlan0", "../INPUTS/dioA-eth1b.pcap", "/dev/null", deb);
        iface->set_debug(deb);
        iface->set_if_index(1);
        iface->set_if_addr(iface_src2);

        iface2 = (pcap_network_interface *)network_interface::find_by_name("wlan0");
        if(!iface2) {
                printf("Did not find if: wlan0\n");
                exit(10);
        }
        printf("iface1[%d]: %s\n", iface->get_if_index(), iface->get_if_name());
        printf("iface2[%d]: %s\n", iface2->get_if_index(), iface2->get_if_name());

        const char *prefixstr = "2001:db8:1::/48";
        ip_subnet prefix;

        err_t e = ttosubnet(prefixstr, strlen(prefixstr),
                            AF_INET6, &prefix);
        iface->set_rpl_prefix(prefix);

        printf("Processing input file\n");
        iface->process_pcap();


	exit(0);
}


