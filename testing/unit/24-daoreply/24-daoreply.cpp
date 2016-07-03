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

        const char * outpcap= "../OUTPUTS/24-node-E-out.pcap";
        dag_network *dag = dioA_setup(deb);
        pcap_network_interface *iface = dioA_iface_setup(dag,deb,outpcap);
        dioA_event(iface, deb, outpcap);

        const char *pcapin2 = "../INPUTS/daoack-A-example661e.pcap";
        iface = daoackA_setup(deb,pcapin2,NULL);
        daoackA_process(iface);

	exit(0);
}


