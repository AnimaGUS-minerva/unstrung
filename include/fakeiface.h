/*
 * This code was originally written by Michael Richardson <mcr@sandelman.ca>
 * as part of the Unstrung project (http://unstrung.sandelman.ca/), and was
 * originally released under the GPL.
 *
 * The original author (me) thinks that this code is generally useful to
 * others who want to use pcap files as sources of testing data, and the
 * rights to this file are hereby (2010-07-01) placed into the *public domain*
 * (for whatever that means),  along with the implementation file pcap_iface.cpp
 *
 * Put your own BSD, MIT, GPL, or all-rights-reserved license on this
 * file as you wish.  You'll need to hack things to make it fit your
 * testing environment, of course.
 *
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
#include <netlink/utils.h>   /* for RT_SCOPE_* */
}

#include "debug.h"


class pcap_network_interface : public network_interface {
public:
  pcap_network_interface(const char *name, rpl_debug *deb);
	pcap_network_interface(pcap_dumper_t *pd);
        ~pcap_network_interface();
        int nisystem(const char *cmd);
        void send_raw_icmp(struct in6_addr *dest, struct in6_addr *src,
                           const unsigned char *icmp_body,
                           const unsigned int icmp_len);
        bool faked(void);

        virtual void skip_pcap_headers(const struct pcap_pkthdr *h,
                                       const u_char *bytes);
        virtual bool setup_lowpan(const unsigned char eui64[8], unsigned int eui64len);
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
                update_addr();
        };

        void setup_infile(const char *infile);
        void setup_outfile(const char *outfile);
        void setup_outfile(const char *outfile, int pcap_link);
        void close(void);

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
                const char *outfile,
                rpl_debug *deb);

        int process_pcap(void);
        void set_link_encap(int link) {
                pcap_link=link;
        };

	void advance_fake_time(void) {
	  rpl_event::advance_fake_time();
	};


        static void fake_linkinfo(const char *new_ifname,
                                  unsigned int myindex,
                                  struct network_interface_init *nii,
                                  unsigned char hwaddr[6],
                                  unsigned int type,
                                  unsigned int flags);

        static void fake_addrinfo(unsigned int myindex,
                                  unsigned int scope,
                                  struct network_interface_init *nii,
                                  unsigned char addr[16]);

protected:
        virtual bool system_get_disable_ipv6(void);
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

        static unsigned int   seq;
};


#endif /* _UNSTRUNG_FAKEIFACE_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
