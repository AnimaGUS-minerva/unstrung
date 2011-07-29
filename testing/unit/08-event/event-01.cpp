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
    rpl_event e1(0, 300, rpl_event::rpl_send_dio, "e1");
    rpl_event e2(0, 500, rpl_event::rpl_send_dio, "e2");
    rpl_event e3(0, 150, rpl_event::rpl_send_dio, "e3");
    rpl_event e4(1, 150, rpl_event::rpl_send_dio, "e4");

    event_map n;

    n[e1.alarm_time] = e1;
    n[e2.alarm_time] = e2;
    n[e3.alarm_time] = e3;
    n[e4.alarm_time] = e4;

    event_map_iterator first = n.begin();
    assert(first != n.end());

    printevents(stdout, n);
    
    rpl_event &n1 = first->second;
    printf("first interval: %u %s\n",
           n1.alarm_time.tv_usec, n1.mReason);

    assert(n1.occurs_in().tv_usec == 150000);

    /* we want the last item, but n.end() is the one after that one */
    /* need to read the STL manual */
    event_map_riterator last = n.rbegin();
    assert(last != n.rend());
    
    rpl_event &n2 = last->second;
    printf("last alarm_time: %u %u %s\n",
           n2.alarm_time.tv_sec,
           n2.alarm_time.tv_usec,
           n2.mReason);
    assert(n2.alarm_time.tv_sec  == 1);
    assert(n2.alarm_time.tv_usec == 150000);
    
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

