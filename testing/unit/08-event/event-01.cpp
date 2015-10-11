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

time_t testedable_now = 1444419263;
struct timeval testable_b = { .tv_sec = 1444419263 };

static void order_test(rpl_event_queue &eq1,
		       rpl_event &e1, rpl_event &e2,
		       rpl_event &e3,
		       rpl_event &e4)
{
    rpl_event *n1;

    /* RECALL: order of events is determined by their timer values! */

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

    /* then it is empty */
    n1 = eq1.next_event();
    assert(n1 == NULL);
}


static void order_test2(rpl_event_queue &eq1,
		       rpl_event &e1, rpl_event &e2,
		       rpl_event &e3,
		       rpl_event &e4)
{
    rpl_event *n1;

    /* RECALL: order of events is determined by their timer values!
     *         e4 was moved to be second.

    /* first event is e3 */
    n1 = eq1.next_event();
    assert(n1 == &e3);

    /* second event is e1 */
    n1 = eq1.next_event();
    assert(n1 == &e1);

    /* third event is e4 */
    n1 = eq1.next_event();
    assert(n1 == &e4);

    /* fourth event is e2 */
    n1 = eq1.next_event();
    assert(n1 == &e2);

    /* then it is empty */
    n1 = eq1.next_event();
    assert(n1 == NULL);
}

/*
 * TEST1: insert four events, see that the youngest comes first.
 */
static void t1(rpl_debug *deb)
{
    rpl_event e1(testable_b, 0,  300, rpl_event::rpl_send_dio, "e1", deb);
    rpl_event e2(testable_b, 5,  500, rpl_event::rpl_send_dio, "e2", deb);
    rpl_event e3(testable_b, 0,  150, rpl_event::rpl_send_dio, "e3", deb);
    rpl_event e4(testable_b, 10, 150, rpl_event::rpl_send_dio, "e4", deb);

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
    assert(e1.inQueue == false);    assert(e2.inQueue == false);
    assert(e3.inQueue == false);    assert(e4.inQueue == false);

    /* now put all the events back on the list in a different order */
    eq1.add_event(&e4);  assert(e4.inQueue == true);
    eq1.add_event(&e3);  assert(e3.inQueue == true);
    eq1.add_event(&e2);  assert(e2.inQueue == true);
    eq1.add_event(&e1);  assert(e1.inQueue == true);
    //eq1.printevents(stdout, "e4-e1   ");

    /* do not clear or test, because we it will remove */
    /* now change one of the timers, and requeue an existing event */
    e4.reset_alarm(testable_b, 0, 400);
    e4.requeue(eq1, testable_b);
    //eq1.printevents(stderr, "resort ");
    order_test2(eq1, e1, e2, e3, e4);
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
 * compile-command: "make check"
 * End:
 */

