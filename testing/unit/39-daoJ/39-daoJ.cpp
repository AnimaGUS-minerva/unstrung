#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"
#include "../24-daoreply/dioA.cpp"
#include "../24-daoreply/daoackA.cpp"

int main(int argc, char *argv[])
{
        pcap_network_interface *iface = NULL;

        rpl_debug *deb = new rpl_debug(true, stdout);
        deb->want_time_log = false;

        dioA_setup(deb);

        const char *pcapin1 = "../INPUTS/24-node-E-dio.pcap";
        const char *outpcap = "../OUTPUTS/39-dao-J.pcap";
        iface = pcap_network_interface::setup_infile_outfile("wlan0",
                                                             pcapin1,
                                                             outpcap, deb);

	struct timeval n;
	n.tv_sec = 1024*1024*1024;
	n.tv_usec = 1024;
	iface->set_fake_time(n);

        struct in6_addr iface_src2;
        inet_pton(AF_INET6, "fe80::1000:ff:fe64:4a01", &iface_src2);
        iface->set_debug(deb);
        iface->set_if_index(1);
        iface->set_if_addr(iface_src2);

        const unsigned char addr[6]={0x10,0x00,0x00,0x64,0x4a,0x01};
        iface->set_eui48(addr, 6);
        iface->watching = true;

        printf("Processing input file %s on if=[%u]: %s state: %s %s\n",
               pcapin1,
               iface->get_if_index(), iface->get_if_name(),
               iface->is_active() ? "active" : "inactive",
               iface->faked() ? " faked" : "");

        iface->process_pcap();

        /* again, drain off any events */
        network_interface::terminating();
        while(network_interface::force_next_event());

	exit(0);
}


