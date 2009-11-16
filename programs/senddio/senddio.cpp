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


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <net/if.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <sys/time.h>
#include <getopt.h>

#include "pathnames.h"
#include "rpl.h"

/* open a raw IPv6 socket, and 
   - send a router advertisement for prefix on argv. (-p)
   - send data from file (in hex)  (-d)
*/

#define HWADDR_MAX 16

#include "hexdump.c"

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

struct Interface {
	char			Name[IFNAMSIZ];	/* interface name */

	struct in6_addr		if_addr;
	unsigned int		if_index;

	uint8_t			init_racount;	/* Initial RAs */

	uint8_t			if_hwaddr[HWADDR_MAX];
	int			if_hwaddr_len;
	int			if_prefix_len;
	int			if_maxmtu;

        int                     rpl_grounded;
        int                     rpl_sequence;
        int                     rpl_instanceid;
        int                     rpl_dagrank;
        unsigned char           rpl_dagid[16];
	uint32_t		AdvLinkMTU;

	time_t			last_multicast_sec;
	suseconds_t		last_multicast_usec;
};


struct Interface ifaceData = {
	.Name          = "eth1",
	.if_index      = 6,
	.if_hwaddr_len = 6,
	.if_hwaddr     = { 0x12, 0x00, 0x00, 0x64, 0x64, 0x23},
	.AdvLinkMTU    = 1500,
};
struct Interface *iface = &ifaceData;

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

int setup_allrouters_membership(int sock, struct Interface *iface)
{
	struct ipv6_mreq mreq;                  
	
	memset(&mreq, 0, sizeof(mreq));                  
	mreq.ipv6mr_interface = iface->if_index;
	
	/* ipv6-allrouters: ff02::2 */
	mreq.ipv6mr_multiaddr.s6_addr32[0] = htonl(0xFF020000);                                          
	mreq.ipv6mr_multiaddr.s6_addr32[3] = htonl(0x2);     

	if (setsockopt(sock, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
	{
		/* linux-2.6.12-bk4 returns error with HUP signal but keep listening */
		if (errno != EADDRINUSE)
		{
			printf("can't join ipv6-allrouters on %s", iface->Name);
			return (-1);
		}
	}

	return (0);
}

int check_allrouters_membership(int sock, struct Interface *iface)
{
	#define ALL_ROUTERS_MCAST "ff020000000000000000000000000002"
	
	FILE *fp;
	unsigned int if_idx, allrouters_ok=0;
	char addr[32+1];
	int ret=0;

	if ((fp = fopen(PATH_PROC_NET_IGMP6, "r")) == NULL)
	{
		printf("can't open %s: %s", PATH_PROC_NET_IGMP6,
			strerror(errno));
		return (-1);	
	}
	
	while ( (ret=fscanf(fp, "%u %*s %32[0-9A-Fa-f] %*x %*x %*x\n", &if_idx, addr)) != EOF) {
		if (ret == 2) {
			if (iface->if_index == if_idx) {
				if (strncmp(addr, ALL_ROUTERS_MCAST, sizeof(addr)) == 0)
					allrouters_ok = 1;
			}
		}
	}

	fclose(fp);

	if (!allrouters_ok) {
		printf("resetting ipv6-allrouters membership on %s\n", iface->Name);
		setup_allrouters_membership(sock, iface);
	}	

	return(0);
}		

void
send_raw_dio(unsigned char *icmp_body, unsigned int icmp_len)
{
	uint8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	struct sockaddr_in6 addr;
	struct in6_addr *dest = NULL;
	struct in6_pktinfo *pkt_info;
	struct msghdr mhdr;
	struct cmsghdr *cmsg;
	struct iovec iov;
	char __attribute__((aligned(8))) chdr[CMSG_SPACE(sizeof(struct in6_pktinfo))];

	int err;

	int sock = open_icmpv6_socket();

	/* Make sure that we've joined the all-routers multicast group */
	if (check_allrouters_membership(sock, iface) < 0)
		printf("problem checking all-routers membership on %s\n", iface->Name);

	printf("sending RA on %u\n", sock);

	if (dest == NULL)
	{
		struct timeval tv;

		dest = (struct in6_addr *)all_hosts_addr;
		gettimeofday(&tv, NULL);

		iface->last_multicast_sec = tv.tv_sec;
		iface->last_multicast_usec = tv.tv_usec;
	}
	
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(IPPROTO_ICMPV6);
	memcpy(&addr.sin6_addr, dest, sizeof(struct in6_addr));

	iov.iov_len  = icmp_len;
	iov.iov_base = (caddr_t) icmp_body;
	
	memset(chdr, 0, sizeof(chdr));
	cmsg = (struct cmsghdr *) chdr;
	
	cmsg->cmsg_len   = CMSG_LEN(sizeof(struct in6_pktinfo));
	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type  = IPV6_PKTINFO;
	
	pkt_info = (struct in6_pktinfo *)CMSG_DATA(cmsg);
	pkt_info->ipi6_ifindex = iface->if_index;
	memcpy(&pkt_info->ipi6_addr, &iface->if_addr, sizeof(struct in6_addr));

#ifdef HAVE_SIN6_SCOPE_ID
	if (IN6_IS_ADDR_LINKLOCAL(&addr.sin6_addr) ||
		IN6_IS_ADDR_MC_LINKLOCAL(&addr.sin6_addr))
			addr.sin6_scope_id = iface->if_index;
#endif

	memset(&mhdr, 0, sizeof(mhdr));
	mhdr.msg_name = (caddr_t)&addr;
	mhdr.msg_namelen = sizeof(struct sockaddr_in6);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (void *) cmsg;
	mhdr.msg_controllen = sizeof(chdr);

	err = sendmsg(sock, &mhdr, 0);
	
	if (err < 0) {
		printf("send_raw_dio/sendmsg: %s\n", strerror(errno));
	}
}


int build_dio(unsigned char *buff, unsigned int buff_len)
{
	uint8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	struct sockaddr_in6 addr;
	struct in6_addr *dest = NULL;
        struct icmp6_hdr  *icmp6;
        struct nd_rpl_dio *dio;
	int len = 0;

	if (dest == NULL)
	{
		struct timeval tv;

		dest = (struct in6_addr *)all_hosts_addr;
		gettimeofday(&tv, NULL);

		iface->last_multicast_sec = tv.tv_sec;
		iface->last_multicast_usec = tv.tv_usec;
	}
	
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(IPPROTO_ICMPV6);
	memcpy(&addr.sin6_addr, dest, sizeof(struct in6_addr));

	memset(buff, 0, buff_len);

        icmp6 = (struct icmp6_hdr *)buff;
	icmp6->icmp6_type = ND_RPL_MESSAGE;
        icmp6->icmp6_code = ND_RPL_DAG_IO;
	icmp6->icmp6_cksum = 0;
        
        dio = (struct nd_rpl_dio *)icmp6->icmp6_data8;

        dio->rpl_flags = 0;
        if(iface->rpl_grounded) {
                dio->rpl_flags |= ND_RPL_DIO_GROUNDED;
        }
        dio->rpl_seq        = iface->rpl_sequence;
        dio->rpl_instanceid = iface->rpl_instanceid;
        dio->rpl_dagrank    = iface->rpl_dagrank;
        memcpy(dio->rpl_dagid, iface->rpl_dagid, 16);

        len = ((caddr_t)&dio[1] - (caddr_t)buff);

        return len;
}

int main(int argc, char *argv[])
{
	int c;
	char *datafilename;
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
                {0,0,0,0},
        };
	
	while((c=getopt_long(argc, argv, "D:I:R:S:Td:p:h?v", longoptions, NULL))!=EOF){
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
			
                case 'T':
                        fakesend=1;
                        break;

                case 'D':
                        if(optarg[0]=='0' && optarg[1]=='x') {
                                char *digits;
                                int i;
                                digits = optarg+2;
                                for(i=0; i<16 && *digits!='\0'; i++) {
                                        unsigned int value;
                                        if(sscanf(digits, "%2x",&value)==0) break;
                                        iface->rpl_dagid[i]=value;

                                        /* advance two characters, carefully */
                                        digits++;
                                        if(digits[0]) digits++;
                                }
                        } else {
                                int len = strlen(optarg);
                                if(len > 16) len=16;

                                memset(iface->rpl_dagid, 0, 16);
                                memcpy(iface->rpl_dagid, optarg, len);
                        }
                        break;

                case 'R':
                        iface->rpl_dagrank  = atoi(optarg);
                        break;

                case 'S':
                        iface->rpl_sequence = atoi(optarg);
                        break;

                case 'I':
                        iface->rpl_instanceid = atoi(optarg);
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
                icmp_len = build_dio(icmp_body, sizeof(icmp_body));
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
                send_raw_dio(icmp_body, icmp_len);
        }

	exit(0);
}
	
