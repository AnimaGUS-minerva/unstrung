/*
 * Copyright (C) 2009-2021 Michael Richardson <mcr@sandelman.ca>
 *
 * SEE LICENSE
 *
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
  //#include "hexdump.c"

#include <netinet/ip6.h>

}

#include "iface.h"
#include "debug.h"
#include "fakeiface.h"
#include "devid.h"

static void usage(void)
{
    fprintf(stderr, "Usage: peck [--verbose] ifname\n");
    fprintf(stderr, "            [--help] \n");
    fprintf(stderr, "            [--fake] \n");
#if defined(HAVE_MBEDTLS)
    fprintf(stderr, "            [--mic=filename] [--privmic=filename] [--manuca=filename]\n");
#endif
    exit(2);
}

int main(int argc, char *argv[])
{
  int verbose = 0;
  bool fakesend=false;
  bool initted =false;
  int c;
  const char *micpemfile = "/boot/mic.pem";
  const char *micprivfile= "/boot/mic.priv";
  const char *manufact_ca= "/boot/manufacturer.pem";
  mbedtls_x509_crt *bootstrap_cert = NULL;
  class rpl_debug *deb;
  char eui64buf[64];
  unsigned char eui64[8];

  struct option longoptions[]={
    {"help",     0, NULL, '?'},
    {"fake",     0, NULL, 'T'},
    {"mic",      1, NULL, 'm'},
    {"privmic",  1, NULL, 'M'},
    {"manuca",   1, NULL, 'R'},
    {"verbose",  0, NULL, 'v'},
    {0,0,0,0},
  };

  deb = new rpl_debug(verbose, stderr);

  while((c=getopt_long(argc, argv, "?hm:vFM:R:V", longoptions, NULL))!=EOF){
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

    case 'R':
      manufact_ca = strdup(optarg);
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

  if(fakesend) {
    pcap_network_interface::scan_devices(deb, false);
  } else {
    network_interface::scan_devices(deb, false);
  }

  device_identity di;
  if(di.load_identity_from_cert( manufact_ca, micpemfile) != 0) {
    exit(10);
  }

  /* extract EUI-64 from certificate */
  int eui64len = di.extract_eui64_from_cert(eui64, eui64buf,
                                            sizeof(eui64buf));
  if(!eui64len == -1) {
    exit(11);
  }

  for(int ifnum = optind; ifnum < argc; ifnum++) {
    class pcap_network_interface *iface = NULL;
    const char *ifname = argv[ifnum];
    iface = (pcap_network_interface *)pcap_network_interface::find_by_name(ifname);

    if(iface == NULL) {
      printf("no such interface: %s", ifname);
      exit(10);
    }
    iface->setup_lowpan(eui64, eui64len);

    /* now send a neighbor solicitation and then start the coap server */
    iface->send_ns(di);
  }

  mbedtls_x509_crt_free( bootstrap_cert );

  exit(0);
}

