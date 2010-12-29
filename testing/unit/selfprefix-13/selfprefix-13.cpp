#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

int main(int argc, char *argv[])
{
        network_interface *iface = NULL;

        rpl_debug *deb = new rpl_debug(true, stderr);

        pcap_network_interface::scan_devices(deb);
        iface = network_interface::find_by_name("wlan0");
        if(!iface) {
                exit(10);
        }

        const char *prefixstr = "2001:db8:1::/48";
        ip_subnet prefix;

        err_t e = ttosubnet(prefixstr, strlen(prefixstr),
                            AF_INET6, &prefix);
        
        iface->set_rpl_prefix(prefix);

	exit(0);
}


