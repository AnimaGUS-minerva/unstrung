/*
 * Copyright (C) 2016 Michael Richardson <mcr@sandelman.ca>
 *
 * SEE FILE COPYING in root of source.
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>
#include <sys/time.h>
#include <getopt.h>

#include "pathnames.h"
#include "oswlibs.h"
#include "rpl.h"

#include "hexdump.c"
}

#include "iface.h"
#include "devid.h"
#include "fakeiface.h"

static void usage(int code)
{
        fprintf(stderr, "Usage: sendnd --solicit [] \n");
        fprintf(stderr, "       sendnd --advert  [] \n");

        exit(code);
}

int main(int argc, char *argv[])
{
    int c;
    char *outfile     = NULL;
    unsigned char icmp_body[2048];
    unsigned int  icmp_len = 0;
    unsigned int verbose=0;
    unsigned int fakesend=0;
    bool advert = false;
    bool solicit= false;

    struct option longoptions[]={
        {"help",     0, NULL, '?'},
        {"debug",    0, NULL, 'v'},
        {"verbose",  0, NULL, 'v'},
        {"fake",     0, NULL, 'T'},
        {"testing",  0, NULL, 'T'},
        {"iface",    1, NULL, 'i'},
        {"outpcap",  1, NULL, 'O'},
        {"advert",   0, NULL, 'a'},
        {"solicit",  0, NULL, 's'},
        {0,0,0,0},
    };

    class rpl_debug *deb = new rpl_debug(verbose, stderr);
    class network_interface *iface = NULL;
    class pcap_network_interface *piface = NULL;
    bool initted = false;
    memset(icmp_body, 0, sizeof(icmp_body));

    while((c=getopt_long(argc, argv, "ashi:vTO:?", longoptions, NULL))!=EOF){
        switch(c) {
        case 'a':
            advert=true;
            break;

        case 's':
            solicit=true;
            break;

        case 'i':
            if(!initted) {
                if(fakesend) {
                    fprintf(stderr, "Using faked interfaces\n");
                    pcap_network_interface::scan_devices(deb, false);
                } else {
                    network_interface::scan_devices(deb, false);
                }
                initted = true;
            }
            iface = network_interface::find_by_name(optarg);
            if(iface && fakesend) {
                if(outfile == NULL) {
                    fprintf(stderr, "Must specify pcap outfile (-O)\n");
                    usage(2);
                }

                if(iface->faked()) {
                    piface = (pcap_network_interface *)iface;
                    piface->set_pcap_out(outfile, DLT_EN10MB);
                } else {
                    fprintf(stderr, "interface %s is not faked\n", optarg);
                    exit(1);
                }
            }
            break;

        case 'T':
            if(initted) {
                fprintf(stderr, "--fake MUST be first argument\n");
                exit(16);
            }
            fakesend=1;
            break;

        case 'O':
            outfile=optarg;
            break;

        case 'v':
            verbose++;
            deb = new rpl_debug(verbose, stderr);
            break;

        case '?':
        case 'h':
        default:
            usage(99);
            break;
        }
    }

    if(!initted || (solicit && advert) || (!solicit && !advert)) {
        fprintf(stderr, "please select either --advert or --solicit\n");
        usage(10);
    }

    device_identity di;

    icmp_len = di.build_neighbour_advert(iface, icmp_body, sizeof(icmp_body));

    if(icmp_len == 0) {
        fprintf(stderr, "length of generated packet is 0, none sent\n");
        usage(1);
    }

    if(verbose) {
        printf("Sending ICMP of length: %u\n", icmp_len);
        if(icmp_len > 0) {
            hexdump(icmp_body, 0, icmp_len);
        }
    }

    struct in6_addr all_hosts_inaddr;
    memcpy(all_hosts_inaddr.s6_addr, all_hosts_addr, 16);

    if(iface != NULL && icmp_len > 0) {
        if(piface != NULL) {
            piface->send_raw_icmp(&all_hosts_inaddr, NULL, icmp_body, icmp_len);
        } else {
            iface->send_raw_icmp(&all_hosts_inaddr, NULL, icmp_body, icmp_len);
        }
    }

    exit(0);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
