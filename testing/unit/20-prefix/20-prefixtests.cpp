/*
 * Unit tests for properties of a prefix.
 */



#include <stdio.h>
#include <stdlib.h>
#include "iface.h"
#include "prefix.h"
extern "C" {
#include "time.h"
}

struct in6_addr dummy_src1;
struct in6_addr dummy_src2;
struct in6_addr dummy_src3;

/*
 * TEST1: a PREFIX can be put into a list.
 */
static void t1(rpl_debug *deb)
{
  prefix_node a1(deb, dummy_src1, 128);
  prefix_node a2(deb, dummy_src2, 128);
  prefix_node a3(deb, dummy_src3, 128);

  prefix_map f1;

  prefix_node &n1 = f1[a1.get_prefix()];
  prefix_node &n2 = f1[a2.get_prefix()];
  prefix_node &n3 = f1[a3.get_prefix()];
}


int main(int argc, char *argv[])
{
  int i;
  
  rpl_debug *deb = new rpl_debug(false, stdout);
  
  inet_pton(AF_INET6, "2001:db8::abcd:00a1", &dummy_src1);
  inet_pton(AF_INET6, "2001:db8::abcd:01a1", &dummy_src2);
  inet_pton(AF_INET6, "2001:db8::abcd:02a1", &dummy_src3);

  //time_t now;
  //time(&now);

  t1(deb);

  exit(0);
}


