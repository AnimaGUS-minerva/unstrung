#include <stdio.h>
#include <stdlib.h>

#include <netinet/ip6.h>

/* open a raw IPv6 socket, and print whatever comes out */
#include "hexdump.c"

main() 
{
	int s = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	char b[2048];
	int len, n;

	if(s < 0) {
		perror("socket");
		exit(10);
	}

	n=0;
	while((len = recv(s, b, 2048, 0)) >= 0) {
		printf("\nPacket(%u,%u):\n", n, len);
		hexdump(b, 0, len);
		n++;
	}
	perror("recv");
}
	
