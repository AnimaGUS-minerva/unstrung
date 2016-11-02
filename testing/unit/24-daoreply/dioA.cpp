#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

dag_network *dioA_setup(rpl_debug *deb)
{
        pcap_network_interface *iface = NULL;
        struct in6_addr iface_src2;

        dag_network *dag = new dag_network(42, "2001:0db8:661e::1", deb);
        dag->set_active();
        dag->set_interval(5000);

        /* now finish setting things up with netlink */
        pcap_network_interface::scan_devices(deb, false);

        return dag;
}

pcap_network_interface *dioA_iface_setup(const char *pcapin1_file,
                                         dag_network *dag,
                                         rpl_debug *deb,
                                         const char *outpcap)
{
        pcap_network_interface *iface = NULL;

        const char *pcapin1 = "../INPUTS/dio-A-661e.pcap";
        if(pcapin1_file == NULL) pcapin1_file = pcapin1;
        iface = pcap_network_interface::setup_infile_outfile("wlan0", pcapin1_file, outpcap, deb);

	struct timeval n;
	n.tv_sec = 1024*1024*1024;
	n.tv_usec = 1024;
	iface->set_fake_time(n);

        struct in6_addr iface_src2;
        inet_pton(AF_INET6, "fe80::1000:ff:fe64:6602", &iface_src2);
        iface->set_debug(deb);
        iface->set_if_index(1);
        iface->set_if_addr(iface_src2);
        iface->watching = true;

        dag->set_debug(deb);

        printf("Processing input file %s on if=[%u]: %s state: %s %s\n",
               pcapin1_file,
               iface->get_if_index(), iface->get_if_name(),
               iface->is_active() ? "active" : "inactive",
               iface->faked() ? " faked" : "");
        return iface;
}

void dioA_event(pcap_network_interface *iface,
                rpl_debug *deb, const char *outpcap)
{
        iface->process_pcap();

        /* now drain off any created events */
        network_interface::terminating();
        while(network_interface::force_next_event());
}

void dioA_step(rpl_debug *deb, const char *outpcap)
{
  dag_network *dag = dioA_setup(deb);
  pcap_network_interface *iface = dioA_iface_setup(NULL, dag, deb,outpcap);
  dioA_event(iface, deb, outpcap);
}


