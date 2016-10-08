#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <net/if_arp.h>
#include <sys/types.h>
}

int main(int argc, char *argv[])
{
        rpl_debug *deb = new rpl_debug(true, stdout);
        deb->want_time_log = false;

        struct network_interface_init nii;

        nii.debug = deb;
        nii.setup = false;

        /* now finish setting things up with netlink */
        pcap_network_interface::scan_devices(deb, false);

        const char *outpcap = "../OUTPUTS/41-acceptns-out.pcap";
        const char *pcapin1 = "../INPUTS/nodeM-ns.pcap";
        pcap_network_interface *iface = NULL;
        char *declinedneighbour = NULL;

        if(argc > 1) pcapin1 = argv[1];
        if(argc > 2) declinedneighbour = argv[2];

        iface = pcap_network_interface::setup_infile_outfile("wlan0",
                                                             pcapin1,
                                                             outpcap, deb);


	struct timeval n;
	n.tv_sec = 1024*1024*1024;
	n.tv_usec = 1024;
	iface->set_fake_time(n);

        if(declinedneighbour) {
          iface->set_neighbour_declined(declinedneighbour, true);
        }

        /* now override our identity from faked out identity */
        /* this needs to be taken from a device identifier certificate */
        struct in6_addr iface_nodeJ;
        inet_pton(AF_INET6, "fe80::1200:ff:fe66:4a02", &iface_nodeJ);
        iface->set_debug(deb);
        iface->set_if_index(1);
        iface->set_if_addr(iface_nodeJ);
        iface->watching = true;

        printf("Processing input file %s on if=[%u]: %s state: %s %s\n",
               pcapin1,
               iface->get_if_index(), iface->get_if_name(),
               iface->is_active() ? "active" : "inactive",
               iface->faked() ? " faked" : "");

#if 0
        /* add interface: eth0 with IPv6: fe80::1000:ff:fe::4a01 for upwards link. */
        unsigned char hwaddr1[6];
        hwaddr1[0]=0x00;        hwaddr1[1]=0x11;
        hwaddr1[2]=0x22;        hwaddr1[3]=0x33;
        hwaddr1[4]=0x44;        hwaddr1[5]=0x45;
        pcap_network_interface::fake_linkinfo("eth0", 10, &nii, hwaddr1,
                                              ARPHRD_ETHER, IFF_BROADCAST);

        ip_address a1;
        ttoaddr("fe80::1000:ff:fe:4a01", 0, AF_INET6, &a1);
        pcap_network_interface::fake_addrinfo(10, RT_SCOPE_UNIVERSE,
                                              &nii, a1.u.v6.sin6_addr.s6_addr);

#endif

        deb->set_debug_flag(RPL_DEBUG_NETINPUT);

        iface->process_pcap();

        /* now drain off any created events */
        network_interface::terminating();
        while(network_interface::force_next_event());

        puts("\n");

        dag_network::print_all_dagstats(stdout, "");

        deb->close_log();
	exit(0);
}


