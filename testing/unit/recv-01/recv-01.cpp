#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

extern "C" {
#include "pcap.h"
#include <libgen.h>
#include <string.h>

}

class pcap_network_interface : public network_interface {
public:
	pcap_network_interface(pcap_dumper_t *pd);
	int send_packet(const u_char *bytes, const int len);

private:
	pcap_dumper_t *pcap_out;
};

pcap_network_interface::pcap_network_interface(pcap_dumper_t *pd) :
	network_interface()
{
	pcap_out = pd;
}

int pcap_network_interface::send_packet(const u_char *bytes, const int len)
{
	struct pcap_pkthdr h;
	memset(&h, 0, sizeof(h));
	h.caplen = len;
	h.len    = len;
	gettimeofday(&h.ts, NULL);
	
	pcap_dump((u_char *)this->pcap_out, &h, bytes);
}


void sunshine_pcap_input(u_char *u,
			 const struct pcap_pkthdr *h,
			 const u_char *bytes)
{
	pcap_network_interface *pnd = (pcap_network_interface *)u;

	pnd->receive_packet(bytes, h->len);
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

	pcap_network_interface *ndproc = new pcap_network_interface(out);
	
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


