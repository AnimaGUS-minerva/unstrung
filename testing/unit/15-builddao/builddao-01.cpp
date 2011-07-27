/*
 * Unit tests for processing a DIO.
 *
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <assert.h>
}

#include "iface.h"
#include "dag.h"
#include "node.h"
#include "dao.h"

#include "fakeiface.h"

/* TEST1: send out a basic DAO */
/* 
 *   To do this we need to have a dag created with a prefix,
 *   and this has to be attached to an interface.
 * 
 *   We create the body that ICMPv6 should send out.
 */
static void t1(rpl_debug *deb)
{
    pcap_network_interface::scan_devices(deb);
}

int main(int argc, char *argv[])
{
    int i;

    rpl_debug *deb = new rpl_debug(true, stderr);
    printf("builddao-01 t1\n");     t1(deb);
    
    printf("builddao-01 tests finished\n");
    exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

