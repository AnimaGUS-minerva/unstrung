#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

int main(int argc, char *argv[])
{
        pcap_network_interface *iface = NULL;

        rpl_debug *deb = new rpl_debug(true, stderr);

        iface = pcap_network_interface::setup_infile_outfile("../INPUTS/dioA-eth1b.pcap","/dev/null");
        iface->set_if_name("wlan0");
        iface->set_if_index(1);

        /* now finish setting things up with netlink */
        pcap_network_interface::scan_devices(deb);

        iface = (pcap_network_interface *)network_interface::find_by_name("wlan0");
        if(!iface) {
                exit(10);
        }

        const char *prefixstr = "2001:db8:1::/48";
        ip_subnet prefix;

        err_t e = ttosubnet(prefixstr, strlen(prefixstr),
                            AF_INET6, &prefix);
        iface->set_rpl_prefix(prefix);

        printf("Processing input file\n");
        iface->process_pcap();
        

	exit(0);
}


