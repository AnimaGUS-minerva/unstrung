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

/* open a raw IPv6 socket, and 
   - send a router advertisement for prefix on argv. (-p)
   - send data from file (in hex)  (-d)
*/

static void usage(void)
{
	fprintf(stderr, "Usage: senddio [-p prefix] [-d datafile]\n");
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

#if 0
int
open_icmpv6_socket(void)
{
	int sock;
	struct icmp6_filter filter;
	int err, val;

        sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (sock < 0)
	{
		printf("can't create socket(AF_INET6): %s", strerror(errno));
		return (-1);
	}

	val = 1;
	err = setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &val, sizeof(val));
	if (err < 0)
	{
		printf("setsockopt(IPV6_RECVPKTINFO): %s", strerror(errno));
		return (-1);
	}

	val = 2;
#ifdef __linux__
	err = setsockopt(sock, IPPROTO_RAW, IPV6_CHECKSUM, &val, sizeof(val));
#else
	err = setsockopt(sock, IPPROTO_IPV6, IPV6_CHECKSUM, &val, sizeof(val));
#endif
	if (err < 0)
	{
		printf("setsockopt(IPV6_CHECKSUM): %s", strerror(errno));
		return (-1);
	}

	val = 255;
	err = setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &val, sizeof(val));
	if (err < 0)
	{
		printf("setsockopt(IPV6_UNICAST_HOPS): %s", strerror(errno));
		return (-1);
	}

	val = 255;
	err = setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &val, sizeof(val));
	if (err < 0)
	{
		printf("setsockopt(IPV6_MULTICAST_HOPS): %s", strerror(errno));
		return (-1);
	}

#ifdef IPV6_RECVHOPLIMIT
	val = 1;
	err = setsockopt(sock, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &val, sizeof(val));
	if (err < 0)
	{
		printf("setsockopt(IPV6_RECVHOPLIMIT): %s", strerror(errno));
		return (-1);
	}
#endif

	/*
	 * setup ICMP filter
	 */
	
	ICMP6_FILTER_SETBLOCKALL(&filter);
	ICMP6_FILTER_SETPASS(ND_ROUTER_SOLICIT, &filter);
	ICMP6_FILTER_SETPASS(ND_ROUTER_ADVERT, &filter);

	err = setsockopt(sock, IPPROTO_ICMPV6, ICMP6_FILTER, &filter,
			 sizeof(filter));
	if (err < 0)
	{
		printf("setsockopt(ICMPV6_FILTER): %s", strerror(errno));
		return (-1);
	}

	return sock;
}

#endif

int main(int argc, char *argv[])
{
	int c;
	const char *datafilename;
	FILE *datafile;
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
                {"dagid",    1, NULL, 'D'},
                {"iface",    1, NULL, 'i'},
                {0,0,0,0},
        };

        class network_interface *iface = new network_interface();
        iface->set_verbose(true, stderr);
	
	while((c=getopt_long(argc, argv, "D:I:R:S:Td:i:h?p:v", longoptions, NULL))!=EOF){
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
                        iface->set_if_name(optarg);
                        break;
			
                case 'T':
                        fakesend=1;
                        break;

                case 'D':
                        iface->set_rpl_dagid(optarg);
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

                icmp_len = iface->build_dio(icmp_body, sizeof(icmp_body), prefix);
        }

	if(verbose) {
                printf("Sending ICMP of length: %u\n", icmp_len);
                if(icmp_len > 0) {
                        hexdump(icmp_body, 0, icmp_len);
                }
	}

        if(icmp_len == 0) {
                usage();
                exit(1);
        }

        if(!fakesend && icmp_len > 0) {
                iface->send_raw_dio(icmp_body, icmp_len);
        }

	exit(0);
}
	
