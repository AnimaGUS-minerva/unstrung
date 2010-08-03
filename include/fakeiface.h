#ifndef _UNSTRUNG_FAKEIFACE_H_
#define _UNSTRUNG_FAKEIFACE_H_

extern "C" {
#include "pcap.h"
#include "sll.h"
#include "ether.h"
#include <libgen.h>
#include <string.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

}

#include "debug.h"

extern int process_infile(char *infile, char *outfile);

class pcap_network_interface : public network_interface {
public:
	pcap_network_interface(pcap_dumper_t *pd);
        ~pcap_network_interface();
        virtual void skip_pcap_headers(const struct pcap_pkthdr *h,
                                       const u_char *bytes);
	int send_packet(const u_char *bytes, const int len);
	void increment_packet(void)   { packet_count++; };
	unsigned int packet_num(void) { return packet_count; };

        static void scan_devices(rpl_debug *deb);

protected:
        void filter_and_receive_icmp6(const time_t now,
                                      const u_char *bytes, int len);

private:

	pcap_dumper_t *pcap_out;
	unsigned int packet_count;
};


#endif /* _UNSTRUNG_FAKEIFACE_H_ */

