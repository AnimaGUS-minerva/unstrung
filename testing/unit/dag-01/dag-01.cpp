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

static void t1(void)
{
        dagid_t d;
        memset(d, 0, DAGID_LEN);
        d[0]='T';
        d[1]='1';
        
        /* a DN shall have a dagid */
        class dag_network dn(d);

        assert(dn.mDagid[0]=='T');
        assert(dn.mDagid[1]=='1');
}

int main(int argc, char *argv[])
{
	int i;

        printf("dag-01 t1\n");
        t1();
	exit(0);
}


