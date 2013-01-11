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


static void order_test(rpl_event_queue &eq1,
		       rpl_event &e1, rpl_event &e2,
		       rpl_event &e3,
		       rpl_event &e4)
{
    rpl_event *n1;

    /* first event is e3 */
    n1 = eq1.next_event();
    assert(n1 == &e3);

    /* second event is e1 */
    n1 = eq1.next_event();
    assert(n1 == &e1);

    /* third event is e2 */
    n1 = eq1.next_event();
    assert(n1 == &e2);

    /* fourth event is e4 */
    n1 = eq1.next_event();
    assert(n1 == &e4);
}

/*
 * TEST1: insert three events, see that the youngest comes first.
 */
static void t1(rpl_debug *deb)
{
    rpl_event e1(0, 300, rpl_event::rpl_send_dio, "e1", deb);
    rpl_event e2(0, 500, rpl_event::rpl_send_dio, "e2", deb);
    rpl_event e3(0, 150, rpl_event::rpl_send_dio, "e3", deb);
    rpl_event e4(1, 150, rpl_event::rpl_send_dio, "e4", deb);

    class rpl_event_queue eq1;

    eq1.add_event(&e1);
    eq1.add_event(&e2);
    eq1.add_event(&e3);
    eq1.add_event(&e4);
    //eq1.printevents(stdout, "e1-e4   ");

    order_test(eq1, e1, e2, e3, e4);

    /* now put all the events back on the list in a different order */
    eq1.add_event(&e2);
    eq1.add_event(&e1);
    eq1.add_event(&e4);
    eq1.add_event(&e3);
    //eq1.printevents(stdout, "swapped ");

    order_test(eq1, e1, e2, e3, e4);

    /* now put all the events back on the list in a different order */
    eq1.add_event(&e4);
    eq1.add_event(&e3);
    eq1.add_event(&e2);
    eq1.add_event(&e1);
    //eq1.printevents(stdout, "e4-e1   ");

    order_test(eq1, e1, e2, e3, e4);
}


int main(int argc, char *argv[])
{
    rpl_debug *deb = new rpl_debug(false, stdout);
    printf("event-01 t1\n");        t1(deb);

    exit(0);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

