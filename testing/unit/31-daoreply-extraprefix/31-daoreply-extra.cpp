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

        class pcap_network_interface *iface = dioA_setup(dag, deb, "../OUTPUTS/29-node-E-out.pcap");

        dioA_process(iface, dag, deb, "../OUTPUTS/29-node-E-out.pcap");

        /* add interface: acp0 with IPv6: 0xfd01:0203:0405:0607::1111/128 */
        iface->set_interface_wildcard("acp*");

        iface = daoackA_setup(deb, "../INPUTS/daoack-A-ripple1.pcap", NULL);
        daoackA_step(deb, iface);

	exit(0);
}


