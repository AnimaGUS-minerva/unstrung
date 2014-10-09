#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

void daoackA_step(rpl_debug *deb,
                  const char *pcapin2, const char *outpcap)
{
        pcap_network_interface *iface = NULL;

        iface = pcap_network_interface::setup_infile_outfile("wlan0", pcapin2, outpcap, deb);
        printf("Processing input file %s on if=[%u]: %s state: %s %s\n",
               pcapin2,
               iface->get_if_index(), iface->get_if_name(),
               iface->is_active() ? "active" : "inactive",
               iface->faked() ? " faked" : "");
        iface->process_pcap();

        /* again, drain off any events */
        network_interface::terminating();
        while(network_interface::force_next_event());
}
