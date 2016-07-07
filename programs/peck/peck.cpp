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
  bool fakesend=false;
  bool initted =false;
  char c;
  char *micpemfile;
  char *micprivfile;
  class rpl_debug *deb;
  struct option longoptions[]={
    {"help",     0, NULL, '?'},
    {"fake",     0, NULL, 'T'},
    {"mic",      1, NULL, 'm'},
    {"privmic",  1, NULL, 'M'},
    {"verbose",  0, NULL, 'v'},
    {0,0,0,0},
  };

  deb = new rpl_debug(verbose, stderr);

  while((c=getopt_long(argc, argv, "?hm:vFM:V", longoptions, NULL))!=EOF){
    switch(c) {
    case 'T':
      if(initted) {
        fprintf(stderr, "--fake MUST be first argument\n");
        exit(16);
      }
      fakesend=true;
      break;

    case 'v':
      verbose++;
      deb->set_verbose(stderr);
      break;

    case 'V':
      fprintf(stderr, "Version: 1.0\n");
      usage();
      break;

    case 'm':
      micpemfile = strdup(optarg);
      break;

    case 'M':
      micprivfile= strdup(optarg);
      break;

    case '?':
    case 'h':
    default:
      usage();
      break;

    }
    initted = true;
  }

  if(!initted) {
    if(fakesend) {
      pcap_network_interface::scan_devices(deb, false);
    } else {
      network_interface::scan_devices(deb, false);
    }
  }

  for(int ifnum = optind; ifnum < argc; ifnum++) {
    class pcap_network_interface *iface = NULL;
    iface = (pcap_network_interface *)pcap_network_interface::find_by_name(argv[ifnum]);

    printf("working on interface: %s\n", argv[ifnum]);
  }


  exit(0);
}

