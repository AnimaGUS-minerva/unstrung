/*
 * Copyright (C) 2009 Michael Richardson <mcr@sandelman.ca>
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
    fprintf(stderr, "Usage: senddio [--verbose] [--prefix prefix]\n");
    fprintf(stderr, "               [-d datafile] [--fake] [--iface net]\n");
    fprintf(stderr, "               [--version #]   [--grounded] [--storing]\n");
    fprintf(stderr, "               [--non-storing] [--multicast] [--no-multicast]\n");
    fprintf(stderr, "               [--sequence #] [--instance #] [--rank #] [--dagid hexstring]\n");

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

bool check_dag(dag_network *dag)
{
    if(dag==NULL) {
	fprintf(stderr, "--dagid must preceed DODAG parameters\n");
	usage();
    }
    return true;
}

int main(int argc, char *argv[])
{
    int c;
    const char *datafilename;
    FILE *datafile;
    char *prefixvalue = NULL;
    char *pcapoutfile = NULL;
    unsigned char icmp_body[2048];
    unsigned int  icmp_len = 0;
    unsigned int verbose=0;
    unsigned int fakesend=0;
    struct option longoptions[]={
        {"fake",     0, NULL, 'T'},
        {"pcapout",  required_argument, NULL,   'w'},
        {"testing",  0, NULL, 'T'},
        {"prefix",   1, NULL, 'p'},
        {"prefixlifetime",   1, NULL, 'P'},
        {"sequence", 1, NULL, 'S'},
        {"instance", 1, NULL, 'I'},
        {"rank",     1, NULL, 'R'},
        {"dagid",    1, NULL, 'D'},
        {"grounded", 0, NULL, 'G'},
        {"storing",        0, NULL, 's'},
        {"non-storing",    0, NULL, 'N'},
        {"multicast",      0, NULL, 'm'},
        {"non-multicast",  0, NULL, 'M'},
        {"iface",    1, NULL, 'i'},
        {"version",  1, NULL, 'V'},
        {"verbose",  0, NULL, 'v'},
        {0,0,0,0},
    };

    class rpl_debug *deb;
    class pcap_network_interface *iface;
    dag_network *dag;
    bool initted = false;
    memset(icmp_body, 0, sizeof(icmp_body));

    deb = new rpl_debug(verbose, stderr);

    while((c=getopt_long(argc, argv, "D:GI:P:R:S:Td:i:h?p:svw:", longoptions, NULL))!=EOF){
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
                    pcap_network_interface::scan_devices(deb);
                } else {
                    network_interface::scan_devices(deb);
                }
                initted = true;
            }
            iface = (pcap_network_interface *)pcap_network_interface::find_by_name(optarg);
            break;

        case 'w': /* --pcapout */
	    pcapoutfile = optarg;

	    /* FALLTHROUGH */
        case 'T':
            if(initted) {
                fprintf(stderr, "--fake MUST be first argument\n");
                exit(16);
            }
            fakesend=1;
            break;

        case 'G':
	    check_dag(dag);
            dag->set_grounded(true);
            break;

        case 'D':
	    dag = new dag_network(optarg);
            break;

        case 'R':
	    check_dag(dag);
            dag->set_dagrank(strtoul(optarg, NULL, 0));
            break;

        case 'S':
	    check_dag(dag);
            dag->set_sequence(strtoul(optarg, NULL, 0));
            break;

        case 'I':
	    check_dag(dag);
            dag->set_instanceid(strtoul(optarg, NULL, 0));
            break;

        case 'p':
            prefixvalue = optarg;
            break;

        case 'P':
	    check_dag(dag);
            dag->set_prefixlifetime(strtoul(optarg, NULL, 0));
            break;

        case 'V':
	    check_dag(dag);
            dag->set_version(strtoul(optarg, NULL, 0));
            break;

        case 's':
	    check_dag(dag);
            dag->set_mode(RPL_DIO_STORING);
            break;

        case 'N':
	    check_dag(dag);
            dag->set_mode(RPL_DIO_NONSTORING);
            break;

        case 'm':
	    check_dag(dag);
            dag->set_nomulticast();
            break;

        case 'M':
	    check_dag(dag);
            dag->set_multicast();
            break;

        case 'v':
            verbose++;
            if(deb) delete deb;
            deb = new rpl_debug(verbose, stderr);
            break;

        case '?':
        case 'h':
        default:
            usage();
            break;
        }
    }

    if(pcapoutfile && iface) {
	iface->set_pcap_out(pcapoutfile, DLT_EN10MB);
    }

    if(prefixvalue) {
        ip_subnet prefix;

        err_t e = ttosubnet(prefixvalue, strlen(prefixvalue),
                            AF_INET6, &prefix);

        icmp_len = dag->build_dio(icmp_body, sizeof(icmp_body), prefix);
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

    if(icmp_len > 0 && (!fakesend || pcapoutfile)) {
        iface->send_raw_icmp(NULL, icmp_body, icmp_len);
    }

    exit(0);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
