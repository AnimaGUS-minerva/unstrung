#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

int main(int argc, char *argv[])
{
	int i;
        rpl_debug *deb = new rpl_debug(true, stderr);
        
        pcap_network_interface::scan_devices(deb);

	/* setup DAG as interesting */
	dag_network *d1 = dag_network::find_or_make_by_string("thisismynicedag1", deb, false);
	d1->set_active();

	for(i = 1; i < argc; i++) {
		char b1[256];
		char *infile = argv[i];
		char *b;
		
		deb->info("PROCESSING %s\n", infile);

		b = basename(infile);
		snprintf(b1, 256, "../OUTPUTS/recv-01-%s", b);
                pcap_network_interface::process_infile("wlan0", infile, b1);
	}
	exit(0);
}


