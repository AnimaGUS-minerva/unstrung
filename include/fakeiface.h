/*
 * This code was originally written by Michael Richardson <mcr@sandelman.ca>
 * as part of the Unstrung project (http://unstrung.sandelman.ca/), and was originally
 * released under the GPL.
 *
 * The original author (me) thinks that this code is generally useful to others who want
 * to use pcap files as sources of testing data, and the rights to this file are hereby
 * (2010-07-01) placed into the *public domain*, along with the implementation file pcap_iface.cpp
 */

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


class pcap_network_interface : public network_interface {
public:
  pcap_network_interface(const char *name, rpl_debug *deb);
	pcap_network_interface(pcap_dumper_t *pd);
        ~pcap_network_interface();
        int nisystem(const char *cmd);
        void send_raw_icmp(struct in6_addr *dest,
                           const unsigned char *icmp_body,
                           const unsigned int icmp_len);
        bool faked(void);

        virtual void skip_pcap_headers(const struct pcap_pkthdr *h,
                                       const u_char *bytes);
	int send_packet(const u_char *bytes, const int len);
	void increment_packet(void)   { packet_count++; };
	unsigned int packet_num(void) { return packet_count; };

        static void scan_devices(rpl_debug *deb, bool setup);
        void set_if_index(int index) {
                if_index = index;
                add_to_list();
        };
        void set_if_addr(struct in6_addr ifa) {
                if_addr = ifa;
        };

        /* a kind of constructor */
        static pcap_network_interface *setup_infile_outfile(
                const char *ifname,
                const char *infile,
                const char *outfile,
		rpl_debug *debug);

        void set_pcap_out(const char *outfile, int pcap_link);
	void close_pcap_files(void);

        static int process_infile(
                const char *ifname,
                const char *infile,
                const char *outfile);

        int process_pcap(void);
        void set_link_encap(int link) {
                pcap_link=link;
        };

	void advance_fake_time(void) {
	  rpl_event::advance_fake_time();
	};


protected:
        void filter_and_receive_icmp6(const time_t now,
                                      const u_char *bytes, int len);

private:
        int            pcap_link;
	pcap_dumper_t *pcap_out;
        pcap_t        *pol;
	unsigned int packet_count;
        void skip_linux_pcap_headers(const struct pcap_pkthdr *h,
                                     const u_char *p);
        void skip_10mb_pcap_headers(const struct pcap_pkthdr *h,
                                    const u_char *p);
};


#endif /* _UNSTRUNG_FAKEIFACE_H_ */

