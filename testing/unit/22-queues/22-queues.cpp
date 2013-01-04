/*
 * Unit tests for priority event queue.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event.h"

extern "C" {
#include <assert.h>
#include <time.h>
}

class rpl_event e1;
class rpl_event e2;
class rpl_event e3;
class rpl_event_queue eq1;

/*
 * TEST1: an event can be put into the list.
 */
static void t1(rpl_debug *deb)
{
  struct timeval n;
  n.tv_sec = 1024*1024*1024;
  n.tv_usec = 1024;
  e1.init_event(n, 120, 9, rpl_event::rpl_send_dio, "test1", deb);
  e2.init_event(n, 60, 7, rpl_event::rpl_send_dao, "test2", deb);
  e3.init_event(n, 1, 2, rpl_event::rpl_send_dio, "test3", deb);

  eq1.add_event(&e1);
  eq1.add_event(&e2);
  eq1.add_event(&e3);

  assert(eq1.queue.size() == 3);

  eq1.printevents(stdout, "");
}

/*
 * TEST2: a NODE can be iterated.
 */
static void t2()
{
  printf("t2:\n");

  class rpl_event *n1;
  n1 = eq1.next_event();
  assert(n1 != NULL);
  n1->printevent(stdout);
  printf("\n");
  eq1.printevents(stdout, "  ");

  n1 = eq1.next_event();
  assert(n1 != NULL);
  n1->printevent(stdout);
  printf("\n");
  eq1.printevents(stdout, "  ");

  n1 = eq1.next_event();
  assert(n1 != NULL);
  n1->printevent(stdout);
  printf("\n");
  eq1.printevents(stdout, "  ");
}

int main(int argc, char *argv[])
{
  rpl_debug *deb = new rpl_debug(false, stdout);
  
  t1(deb);
  t2();

  exit(0);
}


