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

        dioA_step(deb, "../INPUTS/dio-A-661e-ungrounded.pcap", NULL);
        daoackA_step(deb,
                     "../INPUTS/daoack-A-example661e.pcap",
                     "../OUTPUTS/44-node-E-dio.pcap");

	exit(0);
}


