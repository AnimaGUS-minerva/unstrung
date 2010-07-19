/*
 * Unit tests for putting events into a queue, and get them out.
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
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <assert.h>
#include <time.h>
}

#include "iface.h"
#include "event.h"

/*
 * TEST1: insert three events, see that the youngest comes first.
 */
static void t1(void)
{
    rpl_event e1(0, 300, "e1");
    rpl_event e2(0, 500, "e2");
    rpl_event e3(0, 150, "e3");

    event_map n;

    n[e1.interval] = e1;
    n[e2.interval] = e2;
    n[e3.interval] = e3;

    event_map_iterator first = n.begin();
    assert(first != n.end());
    
    rpl_event &n1 = first->second;
    printf("first interval: %u %s\n",
           n1.interval.tv_usec, n1.mReason);
    assert(n1.interval.tv_usec == 150000);
}


int main(int argc, char *argv[])
{
    printf("event-01 t1\n");        t1();

    exit(0);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

