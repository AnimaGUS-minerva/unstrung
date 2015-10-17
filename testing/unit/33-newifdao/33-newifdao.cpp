#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <net/if_arp.h>
#include <sys/types.h>
}

/* refactored because test case 27 uses this */
#include "../24-daoreply/dioA.cpp"
#include "../24-daoreply/daoackA.cpp"

int main(int argc, char *argv[])
{
        rpl_debug *deb = new rpl_debug(true, stdout);
        deb->want_time_log = false;

        struct network_interface_init nii;

        nii.debug = deb;
        nii.setup = false;

        dag_network *dag = new dag_network("ripple", deb);
        dag->set_active();
        dag->set_interval(5000);
        dag->set_interface_wildcard("acp*");
        dag->set_interface_filter("fd01::/16");
        dag->set_ignore_pio(true);

        /* add interface: acp0 with IPv6: fd01:0203:0405:0607::1111/128 */
        unsigned char hwaddr1[6];
        hwaddr1[0]=0x00;        hwaddr1[1]=0x11;
        hwaddr1[2]=0x22;        hwaddr1[3]=0x33;
        hwaddr1[4]=0x44;        hwaddr1[5]=0x45;
        pcap_network_interface::fake_linkinfo("acp0", 10, &nii, hwaddr1, ARPHRD_ETHER, IFF_BROADCAST);

        ip_address a1;
        ttoaddr("fd01:0203:0405:0607::1111", 0, AF_INET6, &a1);
        pcap_network_interface::fake_addrinfo(10, RT_SCOPE_UNIVERSE, &nii, a1.u.v6.sin6_addr.s6_addr);

        /* no other work here: no processing: no packets should be emitted */
        network_interface::terminating();
        while(network_interface::force_next_event());

	exit(0);
}


