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

}

class pcap_network_interface : public network_interface {
public:
	pcap_network_interface(pcap_dumper_t *pd);
        ~pcap_network_interface();
        virtual void skip_pcap_headers(const struct pcap_pkthdr *h,
                                       const u_char *bytes);
	int send_packet(const u_char *bytes, const int len);
	void increment_packet(void)   { packet_count++; };
	unsigned int packet_num(void) { return packet_count; };

protected:
        void filter_and_receive_icmp6(const time_t now,
                                      const u_char *bytes, int len);

private:

	pcap_dumper_t *pcap_out;
	unsigned int packet_count;
};

class pcap_linux_network_interface : public pcap_network_interface {
public:
        virtual void skip_pcap_headers(const struct pcap_pkthdr *h,
                                       const u_char *bytes);
	pcap_linux_network_interface(pcap_dumper_t *pd);
};

pcap_network_interface::~pcap_network_interface() 
{
        pcap_dump_close(pcap_out);
        pcap_out=NULL;
}

pcap_network_interface::pcap_network_interface(pcap_dumper_t *pd) :
	network_interface()
{
	pcap_out = pd;
	packet_count = 0;
}

pcap_linux_network_interface::pcap_linux_network_interface(pcap_dumper_t *pd) :
        pcap_network_interface(pd)
{
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

        pnd->skip_pcap_headers(h, bytes);
}

void pcap_network_interface::skip_pcap_headers(const struct pcap_pkthdr *h,
                                               const u_char *bytes)
{
	int len = h->len;

	/* validate input packet a bit, before passing to receive layer */
	/* first, check ethernet, and reject non EthernetII frames 
	 * from Unit testing.
	 */
	this->increment_packet();

	if(bytes[12]!=0x86 || bytes[13]!=0xdd) {
		printf("packet %u not ethernet II (is: %02x%02x), dropped\n",
		       this->packet_num(), bytes[12], bytes[13]);
		return;
	}
	
	/* make sure it's IPv6! */
	if((bytes[14] & 0xf0) != 0x60) {
		printf("packet %u is not IPv6: %u\n", bytes[14] >> 4);
		return;
	}

	bytes += 14;
	len   -= 14;

        this->filter_and_receive_icmp6(h->ts.tv_sec, bytes, len);
}

void pcap_network_interface::filter_and_receive_icmp6(time_t now,
                                                      const u_char *bytes,
                                                      int len)
{
	struct ip6_hdr *ip6 = (struct ip6_hdr *)bytes;
	unsigned int nh = ip6->ip6_nxt;       /* type of next PDU */

	if(nh != IPPROTO_ICMPV6) {
		printf("packet %u is not ICMPv6, but=proto:%u\n", this->packet_num(), nh);
		return;
	}

	bytes += sizeof(struct ip6_hdr);  /* 40 bytes */
	len   -= sizeof(struct ip6_hdr);

	struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)bytes;
	if(icmp6->icmp6_type != ND_ROUTER_SOLICIT &&
	   icmp6->icmp6_type != ND_ROUTER_ADVERT  &&
	   icmp6->icmp6_type != ND_NEIGHBOR_SOLICIT &&
	   icmp6->icmp6_type != ND_NEIGHBOR_ADVERT  &&
           icmp6->icmp6_type != ND_RPL_MESSAGE) {
		printf("packet %u is not ICMPv6, but=proto:%u\n",
		       this->packet_num(), icmp6->icmp6_type);
		return;
	}

        printf("packet %u is being processed\n", this->packet_num());
	this->receive_packet(ip6->ip6_src, ip6->ip6_dst, now, bytes, len);
}


void pcap_linux_network_interface::skip_pcap_headers(const struct pcap_pkthdr *h,
                                                     const u_char *p)
{
	u_int caplen = h->caplen;
	u_int len = h->len;
	const struct sll_header *sllp;
	u_short ether_type;
	u_short extracted_ethertype;
        const unsigned char *bytes;

	this->increment_packet();

	if (caplen < SLL_HDR_LEN) {
		/*
		 * XXX - this "can't happen" because "pcap-linux.c" always
		 * adds this many bytes of header to every packet in a
		 * cooked socket capture.
		 */
		return;
	}

	sllp = (const struct sll_header *)p;

	/*
	 * Go past the cooked-mode header.
	 */
	len    -= SLL_HDR_LEN;
	caplen -= SLL_HDR_LEN;
	p      += SLL_HDR_LEN;

	ether_type = ntohs(sllp->sll_protocol);

	/*
	 * Is it (gag) an 802.3 encapsulation, or some non-Ethernet
	 * packet type?
	 */
	if (ether_type <= ETHERMTU) {
		/*
		 * Yes - what type is it?
		 */
		switch (ether_type) {
		case LINUX_SLL_P_802_2:
                        bytes = p;
                        /* not correct */
                        
			break;

		default:
                        printf("packet ether_type: %024 rejected\n",
                               ether_type);
                        return;
		}
	} else {
                /* validate input packet a bit, before passing to receive
                 * layer, check ethernet, and reject non EthernetII frames 
                 * from Unit testing.
                 */

                if(ether_type != 0x86dd) {
                        printf("packet %u not ethernet II (is: %04x), dropped\n",
                               this->packet_num(), ether_type);
                        return;
                }
                bytes = p;
        }
	
	/* make sure it's IPv6! */
	if((bytes[0] & 0xf0) != 0x60) {
		printf("packet %u is not IPv6: %u\n", bytes[14] >> 4);
		return;
	}

        this->filter_and_receive_icmp6(h->ts.tv_sec, bytes, len);
}

int process_infile(char *infile, char *outfile)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *pol = pcap_open_offline(infile, errbuf);
        pcap_network_interface *ndproc;

	if(!pol) {
		fprintf(stderr, "can not open input %s: %s\n", infile, errbuf);
		exit(1);
	}

        int pcap_link = pcap_datalink(pol);
	pcap_t *pout = pcap_open_dead(pcap_link, 65535);
	if(!pout) {
		fprintf(stderr, "can not create pcap_open_deads\n");
		exit(1);
	}
		
	pcap_dumper_t *out = pcap_dump_open(pout, outfile);

	if(!out) {
		fprintf(stderr, "can not open output %s\n", outfile);
		exit(1);
	}

        switch(pcap_link) {
        case DLT_LINUX_SLL:
                ndproc = new pcap_linux_network_interface(out);
                break;

        case DLT_EN10MB:
                ndproc = new pcap_network_interface(out);
                break;
                
        default:
                fprintf(stderr, "unimplemented dlt type: %s (%u)",
                        pcap_datalink_val_to_name(pcap_link),pcap_link);
                exit(1);
        }

	ndproc->set_verbose(1, stdout);
	
	pcap_loop(pol, 0, sunshine_pcap_input, (u_char *)ndproc);

	if(ndproc->errors() > 0) {
		exit(ndproc->errors());
	}
}

