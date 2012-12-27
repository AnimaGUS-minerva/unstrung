/*
 * Unit tests for list properties of a node.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

extern "C" {
#include <assert.h>
#include <time.h>
}

struct myfoo {
  char index[32];
};

typedef std::map<const char *, struct myfoo *>          node_map;
typedef node_map::iterator                              node_map_iterator;

node_map nodelist1;
struct myfoo a1;
struct myfoo a2;
struct myfoo a3;

/*
 * TEST1: a NODE can be put into a list.
 */
static void t1(void)
{
  strcpy(a1.index, "2001:db8::abcd:00a1");
  strcpy(a2.index, "2001:db8::abcd:00a2");
  strcpy(a3.index, "2001:db8::abcd:00a3");

  nodelist1[a1.index] = &a1;
  nodelist1[a2.index] = &a2;
  nodelist1[a3.index] = &a3;
}

/*
 * TEST2: a NODE can be iterated.
 */
static void t2()
{
  assert(nodelist1.size() == 3);

  int count=0;
  node_map_iterator nmi = nodelist1.begin();
  while(nmi != nodelist1.end()) {
    struct myfoo *n1 = nmi->second;
    count++;
    nmi++;
  }
  assert(nodelist1.size() == count);
}


int main(int argc, char *argv[])
{
  t1();
  t2();

  exit(0);
}


