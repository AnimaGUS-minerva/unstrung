/*
 * Copyright (C) 2009-2016 Michael Richardson <mcr@sandelman.ca>
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <netinet/ip6.h>

#include "iface.h"
#include "debug.h"
#include "fakeiface.h"

static void usage(void)
{
    fprintf(stderr, "Usage: peck [--verbose] ifname\n");
    exit(2);
}


int main(int argc, char *argv[])
{
  int verbose = 0;
  char c;
  class rpl_debug *deb;
  struct option longoptions[]={
    {"version",  1, NULL, 'V'},
    {"verbose",  0, NULL, 'v'},
    {0,0,0,0},
  };

  deb = new rpl_debug(verbose, stderr);

  while((c=getopt_long(argc, argv, "?hvV", longoptions, NULL))!=EOF){
    switch(c) {
    case 'v':
      verbose++;
      deb->set_verbose(stderr);
      break;

    case 'V':
      fprintf(stderr, "Version: 1.0\n");
      usage();
      break;

    case '?':
    case 'h':
    default:
      usage();
      break;

    }
  }

  exit(0);
}

