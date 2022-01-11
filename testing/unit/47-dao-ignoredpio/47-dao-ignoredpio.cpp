#include <stdio.h>
#include <stdlib.h>
#include "iface.h"
#include "devid.h"

#include "fakeiface.h"
#include "../24-daoreply/dioA.cpp"
#include "../24-daoreply/daoackA.cpp"

int main(int argc, char *argv[])
{
        struct in6_addr iface_src2;
        device_identity di;

        rpl_debug *deb = new rpl_debug(true, stdout);
        deb->want_time_log = false;

        int loaded = di.load_identity_from_cert(NULL, "../45-extract-eui/f202.crt");

        if(!di.parse_rfc8994cert()) {
          fprintf(stderr, "failed to parse certificate\n");
          exit(4);
        }

        const char *infile1 = "../INPUTS/dio-A-661e-ungrounded.pcap";
        const char *outpcap = NULL;

        dag_network *dag = dioA_setup(deb);

        dag->set_acp_identity(&di);
        dag->set_ignore_pio(true);
        dag->add_all_interfaces();

        pcap_network_interface *iface = dioA_iface_setup(infile1, dag, deb,outpcap);
        dioA_event(iface, deb, outpcap);

        daoackA_step(deb,
                     "../INPUTS/daoack-A-example661e.pcap",
                     "../OUTPUTS/47-node-E-dio.pcap");

	exit(0);
}


