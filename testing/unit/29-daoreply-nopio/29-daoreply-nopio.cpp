#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

/* refactored because test case 27 uses this */
#include "../24-daoreply/dioA.cpp"
#include "../24-daoreply/daoackA.cpp"

int main(int argc, char *argv[])
{
        rpl_debug *deb = new rpl_debug(true, stdout);
        deb->want_time_log = false;

        dag_network *dag = dioA_setup(deb);

        dag->set_ignore_pio(true);

        const char *outpcap = "../OUTPUTS/29-node-E-out.pcap";
        pcap_network_interface *iface = dioA_iface_setup(dag,deb,outpcap);
        dioA_event(iface, deb, outpcap);
        daoackA_step(deb, "../INPUTS/daoack-A-example661e.pcap", NULL);

	exit(0);
}


