#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

int main(int argc, char *argv[])
{
        pcap_network_interface *iface = NULL;
        pcap_network_interface *iface2 = NULL;
        struct in6_addr iface_src2;

	//	rpl_event::event_debug_time = true;

        rpl_debug *deb = new rpl_debug(true, stderr);
        inet_pton(AF_INET6, "fe80::1000:ff:fe64:6423", &iface_src2);

        dag_network *d1 = new dag_network(42, "2001:0db8:661e::1", deb);

	d1->set_active();
	d1->set_interval(50);

        /* now finish setting things up with netlink */
        pcap_network_interface::scan_devices(deb, false);

	/* this has a InstanceID: 42 */
        iface = pcap_network_interface::setup_infile_outfile("wlan0",
							     "../INPUTS/dio-A-661e.pcap",
							     "../OUTPUTS/19-pickdag.pcap", deb);
        iface->set_debug(deb);

	struct timeval n;
	n.tv_sec = 1024*1024*1024;
	n.tv_usec = 1024;
	iface->set_fake_time(n);
        iface->set_if_index(1);
        iface->set_if_addr(iface_src2);

        deb->log("Processing input file 1\n");
        iface->process_pcap();

        deb->log("Events created for file 1\n");
	iface->things_to_do.printevents(stderr, "");

        deb->log("Forcing event1 created for file 1\n");
	iface->advance_fake_time();
	iface->force_next_event();
	iface->things_to_do.printevents(stderr, "");

        deb->log("Forcing event2 created for file 1\n");
	iface->advance_fake_time();
	iface->force_next_event();
	iface->things_to_do.printevents(stderr, "");

	iface->close_pcap_files();
	iface->clear_events();

	/* this has a DAGID: T2 (should not be processed */
        iface = pcap_network_interface::setup_infile_outfile("wlan0",
                                                             "../INPUTS/dio-B-661e.pcap",
                                                             "/dev/null", deb);

        deb->log("\nProcessing input file 2\n");
        iface->process_pcap();

        deb->log("Events created for file 2 (should be none)\n");
	iface->things_to_do.printevents(stderr, "");

        deb->log("Forcing events created for file 2 (should be none)\n");
	iface->force_next_event();


	exit(0);
}


