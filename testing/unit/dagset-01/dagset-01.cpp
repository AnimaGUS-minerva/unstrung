/*
 * Unit tests for processing a DIO.
 *
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <assert.h>
}

#include "iface.h"
#include "dag.h"
#include "node.h"

/* TEST1: instantiate an RPL_node */
static void t1(void)
{
        class rpl_node *n = new rpl_node;
        delete n;
}

int main(int argc, char *argv[])
{
	int i;


        printf("dagset-01 t1\n");     t1();
	exit(0);
}


