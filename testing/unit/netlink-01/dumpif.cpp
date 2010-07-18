#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <assert.h>

#include "netlink/libnetlink.h"
}

#include "iface.h"
#include "dag.h"

/*
 * this program needs to dump the list of interfaces via netlink,
 * printing the MAC address of each interface that it sees.
 */

struct rtnl_handle my_rth;
const char * _SL_ = "dumpif";

extern int print_linkinfo(const struct sockaddr_nl *who, 
                          struct nlmsghdr *n, void *arg);


int main(int argc, char *argv[])
{
    int i;
    int preferred_family = AF_PACKET;

    if (rtnl_open(&my_rth, 0) < 0) {
            fprintf(stderr, "Cannot open rtnetlink\n");
            return -1;
    }

    if (rtnl_wilddump_request(&my_rth, preferred_family, RTM_GETLINK) < 0) {
        perror("Cannot send dump request");
        exit(1);
    }
    
    if (rtnl_dump_filter(&my_rth, print_linkinfo, stdout, NULL, NULL) < 0) {
        fprintf(stderr, "Dump terminated\n");
        exit(1);
    }

    exit(0);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */


