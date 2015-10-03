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

        if(argc != 3) {
          printf("usage: 30-ackdao <dao1.pcap> <doa2.pcap>\n");
          exit(10);
        }

        iface = pcap_network_interface::setup_infile_outfile("wlan0",
                                                             argv[1],
                                                             "../OUTPUTS/30-node-A-out.pcap",
                                                             deb);
	struct timeval n;
	n.tv_sec = 1024*1024*1024;
	n.tv_usec = 1024;
	iface->set_fake_time(n);

        iface->set_debug(deb);
        iface->set_if_index(1);
        iface->set_if_addr(iface_src2);
        printf("iface1[%d]: %s\n", iface->get_if_index(), iface->get_if_name());

        const char *prefixstr = "2001:db8:1::/48";
        ip_subnet prefix;

	dag_network *dn = iface->find_or_make_dag_by_dagid("ripple");

        err_t e = ttosubnet(prefixstr, strlen(prefixstr),
                            AF_INET6, &prefix);
        dn->set_prefix(prefix);
        dn->addselfprefix(iface);
	dn->set_active();
	dn->set_debug(deb);

        printf("Processing input file\n");
        iface->process_pcap();


	exit(0);
}


