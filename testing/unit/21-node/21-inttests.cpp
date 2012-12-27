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

typedef std::map<int, int>          node_map;
typedef node_map::iterator          node_map_iterator;


int main(int argc, char *argv[])
{
  node_map nodelist1;

  nodelist1[10]=100;
  nodelist1[20]=400;
  nodelist1[30]=900;

  assert(nodelist1.size() == 3);

  int count=0;
  node_map_iterator nmi = nodelist1.begin();
  while(nmi != nodelist1.end()) {
    assert(nmi->first * nmi->first == nmi->second);
    count++;
    nmi++;
  }
  assert(nodelist1.size() == count);

  exit(0);
}


