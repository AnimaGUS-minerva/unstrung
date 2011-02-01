/*
 * Copyright (C) 2010 Michael Richardson <mcr@sandelman.ca>
 */

/*
 * parts of this file are derived from send.c of radvd, by 
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <pekkas@netcore.fi>.
 *
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
#include "fakeiface.h"

/* open a raw IPv6 socket, and 
   - send a router advertisement for prefix on argv. (-p)
   - send data from file (in hex)  (-d)
*/

static void usage(void)
{
    fprintf(stderr, "Usage: senddao [--prefix prefix] [-d datafile] [--fake] [--iface net]\n");
    fprintf(stderr, "               [--sequence #] [--instance #] [--rank #] [--dagid hexstring]\n");
    fprintf(stderr, "               [--outpcap file] \n");

    exit(2);
}

unsigned int read_hex_values(FILE *in, unsigned char *buffer)
{
    int count = 0;
    unsigned int data;
    int c;

    while((c = fgetc(in)) != EOF) {
        if(c == '#') {
            /* skip comment */
            while((c = fgetc(in)) != EOF &&
                  c != '\n');
            if(c==EOF) return count;
            continue;
        }
        ungetc(c, in);
        while(fscanf(in, "%2x", &data) > 0) {
            buffer[count++]=data;
        }
    }
    return count;
}

int main(int argc, char *argv[])
{
    int c;
    const char *datafilename;
    FILE *datafile;
    char *outfile     = NULL;
    char *prefixvalue = NULL;
    unsigned char icmp_body[2048];
    unsigned int  icmp_len = 0;
    unsigned int verbose=0;
    unsigned int fakesend=0;
    struct option longoptions[]={
        {"fake",     0, NULL, 'T'},
        {"testing",  0, NULL, 'T'},
        {"prefix",   1, NULL, 'p'},
        {"sequence", 1, NULL, 'S'},
        {"instance", 1, NULL, 'I'},
        {"rank",     1, NULL, 'R'},
        {"dagid",    1, NULL, 'G'},
        {"iface",    1, NULL, 'i'},
        {"outpcap",  1, NULL, 'O'},
        {0,0,0,0},
    };

    class rpl_debug *deb;
    class network_interface *iface = NULL;
    class dag_network       *dn = NULL;
    class pcap_network_interface *piface = NULL;
    bool initted = false;
    memset(icmp_body, 0, sizeof(icmp_body));
	
    while((c=getopt_long(argc, argv, "D:G:I:O:R:S:Td:i:h?p:v", longoptions, NULL))!=EOF){
        switch(c) {
        case 'd':
            datafilename=optarg;
            if(datafilename[0]=='-' && datafilename[1]=='\0') {
                datafile = stdin;
                datafilename="<stdin>";
            } else {
                datafile = fopen(datafilename, "r");
            }
            if(!datafile) {
                perror(datafilename);
                exit(1);
            }
            icmp_len = read_hex_values(datafile, icmp_body);
            break;

        case 'i':
            if(!initted) {
                if(fakesend) {
                    fprintf(stderr, "Using faked interfaces\n");
                    pcap_network_interface::scan_devices(deb);
                } else {
                    network_interface::scan_devices(deb);
                }
                initted = true;
            }
            if(outfile == NULL) {
                    fprintf(stderr, "Must specify pcap outfile (-O)\n");
                    exit(2);
            }

            iface = network_interface::find_by_name(optarg);
            if(iface && fakesend) {
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

        case 'G':
            dn = iface->find_or_make_dag_by_dagid(optarg);
            break;

        case 'R':
            iface->set_rpl_dagrank(atoi(optarg));
            break;

        case 'S':
            iface->set_rpl_sequence(atoi(optarg));
            break;

        case 'I':
            iface->set_rpl_instanceid(atoi(optarg));
            break;

        case 'p':
            prefixvalue = optarg;
            break;

        case 'v':
            verbose++;
            deb = new rpl_debug(verbose, stderr);
            break;

        case '?':
        case 'h':
        default:
            usage();
            break;
        }
    }

    if(prefixvalue) {
        ip_subnet prefix;

        err_t e = ttosubnet(prefixvalue, strlen(prefixvalue),
                            AF_INET6, &prefix);

        //icmp_len = iface->build_dao(icmp_body, sizeof(icmp_body), prefix);
    }

    if(icmp_len == 0) {
        usage();
        exit(1);
    }

    if(verbose) {
        printf("Sending ICMP of length: %u\n", icmp_len);
        if(icmp_len > 0) {
            hexdump(icmp_body, 0, icmp_len);
        }
    }

    if(iface != NULL && icmp_len > 0) {
        if(piface != NULL) {
            piface->send_raw_icmp(icmp_body, icmp_len);
        } else {
            iface->send_raw_icmp(icmp_body, icmp_len);
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
