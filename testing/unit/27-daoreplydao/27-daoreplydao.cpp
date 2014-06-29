#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"
#include "../24-daoreply/dioA.cpp"
#include "../24-daoreply/daoackA.cpp"

int main(int argc, char *argv[])
{
        pcap_network_interface *iface = NULL;
        struct in6_addr iface_src2;

        rpl_debug *deb = new rpl_debug(true, stdout);
        deb->want_time_log = false;

        dioA_step(deb, NULL);
        daoackA_step(deb, "../OUTPUTS/27-node-E-dio.pcap");

        const char *pcapin3 = "../INPUTS/27-daoJ.pcap";
        iface = pcap_network_interface::setup_infile_outfile("wlan0", pcapin3, "../OUTPUTS/27-daodaoack.pcap", deb);

        printf("Processing input file %s on if=[%u]: %s state: %s %s\n",
               pcapin3,
               iface->get_if_index(), iface->get_if_name(),
               iface->is_active() ? "active" : "inactive",
               iface->faked() ? " faked" : "");
        iface->process_pcap();

        /* again, drain off any events */
        network_interface::terminating();
        while(network_interface::force_next_event());

	exit(0);
}


