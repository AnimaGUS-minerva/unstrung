#include <stdio.h>
#include <stdlib.h>
#include "ndmgmt.h"

extern "C" {
#include "pcap.h"
#include <libgen.h>

#if 0
	external void sunshine_pcap_input(u_char *u,
					  const struct pcap_pkthdr *h,
					  const u_char *bytes);
#endif
}

class pcap_neighbour_discovery : public neighbour_discovery {
public:
	pcap_neighbour_discovery(pcap_dumper_t *pd);
	pcap_dumper_t *pcap_out;

private:
};

pcap_neighbour_discovery::pcap_neighbour_discovery(pcap_dumper_t *pd) :
	neighbour_discovery()
{
	pcap_out = pd;
}

void sunshine_pcap_input(u_char *u,
			 const struct pcap_pkthdr *h,
			 const u_char *bytes)
{
	pcap_neighbour_discovery *pnd = (pcap_neighbour_discovery *)u;

	pcap_dump((u_char *)pnd->pcap_out, h, bytes);
}

int process_infile(char *infile, char *outfile)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *pol = pcap_open_offline(infile, errbuf);

	if(!pol) {
		fprintf(stderr, "can not open input %s: %s\n", infile, errbuf);
		exit(1);
	}

	pcap_t *pout = pcap_open_dead(pcap_datalink(pol), 65535);
	if(!pout) {
		fprintf(stderr, "can not create pcap_open_deads\n");
		exit(1);
	}
		
	pcap_dumper_t *out = pcap_dump_open(pout, outfile);

	if(!out) {
		fprintf(stderr, "can not open output %s\n", outfile);
		exit(1);
	}

	pcap_neighbour_discovery *ndproc = new pcap_neighbour_discovery(out);
	
	pcap_loop(pol, 0, sunshine_pcap_input, (u_char *)ndproc);

	if(ndproc->errors() > 0) {
		exit(ndproc->errors());
	}
}

int main(int argc, char *argv[])
{
	int i;

	for(i = 1; i < argc; i++) {
		char b1[256];
		char *infile = argv[i];
		char *b;
		
		b = basename(infile);
		snprintf(b1, 256, "../OUTPUTS/recv-01-%s", b);
		process_infile(infile, b1);
	}
	exit(0);
}


