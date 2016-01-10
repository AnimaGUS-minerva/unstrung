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

        iface = pcap_network_interface::setup_infile_outfile("wlan0",
                                                             "../INPUTS/35-nodeE-dis.pcap",
                                                             "../OUTPUTS/35-node-A-out.pcap",
                                                             deb);
	struct timeval n;
	n.tv_sec = 1024*1024*1024;
	n.tv_usec = 1024;
	iface->set_fake_time(n);

        iface->set_debug(deb);
        iface->set_if_index(1);
        iface->set_if_addr(iface_src2);

	dag_network *dn = dag_network::find_or_make_by_instanceid(2,
                                                                  iface_src2,
                                                                  deb, false);
        deb->set_debug_flag(RPL_DEBUG_NETINPUT);
        printf("Processing input file\n");
        iface->process_pcap();

        deb->flush();

	exit(0);
}


