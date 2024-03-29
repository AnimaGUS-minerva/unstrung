/*
 * This code was originally written by Michael Richardson <mcr@sandelman.ca>
 * as part of the Unstrung project, and was under the GPL.
 *
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * The original author (me) thinks that this code is generally useful to
 * others who want to use pcap files as sources of testing data, and the
 * rights to this file are hereby
 * (2010-07-01) placed into the *public domain*, along with the header
 * file fakeiface.h.
 */

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
#include <unistd.h>
#include "hexdump.c"
}

#include <netlink/rt_names.h>
#include <netlink/utils.h>
#include <netlink/ll_map.h>

unsigned wlan1_is_down = 0;

unsigned int pcap_network_interface::seq = 0;

class pcap_iface_factory : public iface_factory {
public:
    virtual network_interface *newnetwork_interface(const char *name,
						    rpl_debug *deb);
};

network_interface *pcap_iface_factory::newnetwork_interface(const char *name,
							    rpl_debug *deb)
{
    return new pcap_network_interface(name, deb);
}

/* new factory / creates pcap_network_interface */
class pcap_iface_factory pcap_factory;


/* constructor */
pcap_network_interface::pcap_network_interface(const char *name, rpl_debug *deb) :
    network_interface(name, deb)
{
    if(debug) {
	debug->verbose("Creating PCAP interface: %s\n", name);
    }
    alive = true;
}

pcap_network_interface::~pcap_network_interface()
{
    close();
}

pcap_network_interface::pcap_network_interface(pcap_dumper_t *pd) :
	network_interface()
{
	pcap_out = pd;
	packet_count = 0;
}

/* faked interfaces always have IPv6, or we wouldn't bother, would we? */
bool pcap_network_interface::system_get_disable_ipv6(void)
{
    return false;
}

bool pcap_network_interface::faked(void) {
    return true;
};

void pcap_network_interface::close(void) {
    if(pcap_out) pcap_dump_close(pcap_out);
    pcap_out=NULL;
};

void
pcap_network_interface::send_raw_icmp(struct in6_addr *dest,
                                      struct in6_addr *src,
                                      const unsigned char *icmp_body,
                                      const unsigned int icmp_len)
{
    uint8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
    unsigned char packet[icmp_len+sizeof(ip6_hdr)+16];

    if (dest == NULL)
    {
        dest = (struct in6_addr *)all_hosts_addr;  /* XXX WRONG WRONG WRONG */
    }

    /* layer 2 */
    memset(packet, 0, sizeof(packet));
    packet[0] =0x02; packet[1]=0x34; packet[2]=0x56;
    packet[3] =0x78; packet[4]=0x9a; packet[5]=0xbc;

    memcpy(packet+6, this->if_hwaddr, 6);

    packet[12]=0x86;
    packet[13]=0xdd;

    /* layer 3 */
    unsigned char *layer3 = &packet[14];
    struct ip6_hdr *v6 = (struct ip6_hdr *)layer3;

    this->get_link_local(&v6->ip6_src);
    if(src) {
        memcpy(&v6->ip6_src, src, 16);
    }
    v6->ip6_vfc = 6 << 4;
    memcpy(&v6->ip6_dst, dest, 16);
    v6->ip6_nxt = IPPROTO_ICMPV6;
    v6->ip6_hlim= 64;
    v6->ip6_plen= htons(icmp_len);

    /* layer 4+ */
    unsigned char *payload = (unsigned char *)(v6+1);
    memcpy(payload, icmp_body, icmp_len);

    struct icmp6_hdr *icmp6h = (struct icmp6_hdr *)payload;

    /* compute checksum */
    icmp6h->icmp6_cksum = 0;
    unsigned short icmp6sum = csum_ipv6_magic(&v6->ip6_src,
                                          &v6->ip6_dst,
                                          icmp_len, IPPROTO_ICMPV6,
                                          0);

#if 0
    unsigned int chksum_len = icmp_len+sizeof(struct ip6_hdr);
    printf("iface output of %u bytes (%u)\n", chksum_len, icmp_len);
    hexdump(layer3, 0, chksum_len);
#endif
    icmp6sum = csum_partial(payload, icmp_len, icmp6sum);
    icmp6sum = (~icmp6sum & 0xffff);
    icmp6h->icmp6_cksum = icmp6sum;

    struct pcap_pkthdr h;
    memset(&h, 0, sizeof(h));
    h.caplen = 14+40+icmp_len;
    h.len    = h.caplen;
    gettimeofday(&h.ts, NULL);

    if(this->pcap_out) {
        pcap_dump((u_char *)this->pcap_out, &h, packet);
    }
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
    switch(pcap_link) {
    case DLT_LINUX_SLL:
        skip_linux_pcap_headers(h, bytes);
        break;

    case DLT_EN10MB:
        skip_10mb_pcap_headers(h, bytes);
        break;

    default:
        debug->info("unimplemented dlt type: %s (%u)",
		    pcap_datalink_val_to_name(pcap_link),pcap_link);
        exit(1);
    }
}


void pcap_network_interface::skip_10mb_pcap_headers(const struct pcap_pkthdr *h,
                                                    const u_char *bytes)
{
	int len = h->len;

	/* validate input packet a bit, before passing to receive layer */
	/* first, check ethernet, and reject non EthernetII frames
	 * from Unit testing.
	 */
	this->increment_packet();

	if(bytes[12]!=0x86 || bytes[13]!=0xdd) {
		debug->warn("packet %u not ethernet II (is: %02x%02x), dropped\n",
                            this->packet_num(), bytes[12], bytes[13]);
		return;
	}

	/* make sure it's IPv6! */
	if((bytes[14] & 0xf0) != 0x60) {
		debug->warn("packet %u is not IPv6: %u\n", bytes[14] >> 4);
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
		debug->warn("packet %u is not ICMPv6, but=proto:%u\n", this->packet_num(), nh);
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
		debug->warn("packet %u is not ICMPv6, but=proto:%u\n",
		       this->packet_num(), icmp6->icmp6_type);
		return;
	}

        debug->warn("packet %u is being processed\n", this->packet_num());
	this->receive_packet(ip6->ip6_src, ip6->ip6_dst, now, bytes, len);
}


void pcap_network_interface::skip_linux_pcap_headers(const struct pcap_pkthdr *h,
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
                        debug->warn("packet ether_type: %024 rejected\n",
                               ether_type);
                        return;
		}
	} else {
                /* validate input packet a bit, before passing to receive
                 * layer, check ethernet, and reject non EthernetII frames
                 * from Unit testing.
                 */

                if(ether_type != 0x86dd) {
                        debug->warn("packet %u not ethernet II (is: %04x), dropped\n",
                               this->packet_num(), ether_type);
                        return;
                }
                bytes = p;
        }

	/* make sure it's IPv6! */
	if((bytes[0] & 0xf0) != 0x60) {
		debug->warn("packet %u is not IPv6: %u\n", bytes[14] >> 4);
		return;
	}

        this->filter_and_receive_icmp6(h->ts.tv_sec, bytes, len);
}

struct fake_device_msg {
    struct nlmsghdr  nlh;
    struct ifinfomsg if1;
    struct rtattr    ifname;
    unsigned char    ifnamespace[256];
    struct rtattr    ifmtu;
    unsigned int     ifmtu_value;
};

void pcap_network_interface::fake_linkinfo(const char *new_ifname,
                                           unsigned int myindex,
                                           struct network_interface_init *nii,
                                           unsigned char eui[6],
                                           unsigned int type,
                                           unsigned int flags)
{
    struct sockaddr_nl who;
    struct fake_device_msg fake1;
    struct nlmsghdr *nlh = &fake1.nlh;
    struct ifinfomsg *ifi= (struct ifinfomsg *)NLMSG_DATA(nlh);
    struct rtattr    *rtname = IFLA_RTA(ifi);

    int    len      = 0;

    nlh->nlmsg_type  = RTM_NEWLINK;
    nlh->nlmsg_flags = 0;  /* not sure what to set here */
    nlh->nlmsg_seq   = ++seq;
    nlh->nlmsg_pid   = getpid();

    ifi->ifi_index   = myindex;
    ifi->ifi_type    = type;   /* ARPHRD_ETHER; */
    //ifi->ifi_family  = PF_ETHER;
    ifi->ifi_flags   = flags;  /* IFF_BROADCAST; */

    rtname->rta_type = IFLA_IFNAME;
    char *ifname = (char *)RTA_DATA(rtname);
    ifname[0]='\0';
    strncat(ifname, new_ifname, sizeof(fake1.ifnamespace));
    rtname->rta_len  = RTA_LENGTH(strlen(ifname)+1);

    struct rtattr *rtmtu = RTA_NEXT(rtname, len);
    rtmtu->rta_type = IFLA_MTU;
    unsigned int *mtu = (unsigned int *)RTA_DATA(rtmtu);
    *mtu            = 1500;
    rtmtu->rta_len  = RTA_LENGTH(sizeof(int));

    struct rtattr *rtaddr = RTA_NEXT(rtmtu, len);
    rtaddr->rta_type= IFLA_ADDRESS;
    unsigned char *hwaddr = (unsigned char *)RTA_DATA(rtaddr);
    memcpy(hwaddr, eui, 6);
    rtaddr->rta_len = RTA_LENGTH(6);

    struct rtattr *rtlast = RTA_NEXT(rtaddr, len);

    nlh->nlmsg_len  = NLMSG_LENGTH(sizeof(*ifi)) + (-len);
    gather_linkinfo(&who, (struct nlmsghdr *)&fake1, (void*)nii);
}

void pcap_network_interface::fake_addrinfo(unsigned int myindex,
                                           unsigned int scope,
                                           struct network_interface_init *nii,
                                           unsigned char addr[16])
{
    struct sockaddr_nl who;
    struct fake_device_msg fake1;
    struct nlmsghdr *nlh = &fake1.nlh;
    struct ifaddrmsg *iai= (struct ifaddrmsg *)NLMSG_DATA(nlh);
    struct rtattr    *rtaddr6 = IFA_RTA(iai);
    int    len      = 0;
    nlh->nlmsg_type  = RTM_NEWADDR;
    nlh->nlmsg_flags = 0;  /* not sure what to set here */
    nlh->nlmsg_seq   = ++seq;
    nlh->nlmsg_pid   = getpid();

    iai->ifa_family  = AF_INET6;
    iai->ifa_prefixlen = 64;
    iai->ifa_flags   = IFA_F_PERMANENT;
    iai->ifa_scope   = scope;
    iai->ifa_index   = myindex;

    unsigned char *addr6 = (unsigned char *)RTA_DATA(rtaddr6);
    memcpy(addr6, addr, 16);
    rtaddr6->rta_type= IFA_LOCAL;
    rtaddr6->rta_len = RTA_LENGTH(16);

    struct rtattr *rtlast = RTA_NEXT(rtaddr6, len);
    nlh->nlmsg_len  = NLMSG_LENGTH(sizeof(*iai)) + (-len);
    gather_linkinfo(&who, (struct nlmsghdr *)&fake1, (void*)nii);
}


void pcap_network_interface::scan_devices(rpl_debug *deb, bool setup)
{
        /* fix up the factory */
        iface_maker = &pcap_factory;

        struct sockaddr_nl who;
        unsigned int ifindex = 0;

        struct network_interface_init nii;
        struct fake_device_msg fake1, fake2;

        nii.debug = deb;
        nii.setup = setup;

        memset(&who, 0, sizeof(who));
        int myindex = ++ifindex;

        /* make a wlan0 interface */
        {
            unsigned char hwaddr[6];
            hwaddr[0]=0x00;        hwaddr[1]=0x16;
            hwaddr[2]=0x3e;        hwaddr[3]=0x11;
            hwaddr[4]=0x34;        hwaddr[5]=0x24;

            fake_linkinfo("wlan0", myindex, &nii, hwaddr,
                          ARPHRD_ETHER, IFF_BROADCAST|IFF_UP|IFF_RUNNING);
        }

        /* now send up the IPv6 address for a wlan0 address */
        {
            unsigned char addr6[16];
            addr6[0] = 0xfe;                addr6[1] = 0x80;
            addr6[2] = 0x00;                addr6[3] = 0x00;
            addr6[4] = 0x00;                addr6[5] = 0x00;
            addr6[6] = 0x00;                addr6[7] = 0x00;
            addr6[8] = 0x10;                addr6[9] = 0x00;
            addr6[10]= 0x00;                addr6[11]= 0xff;
            addr6[12]= 0xfe;                addr6[13]= 0x64;
            addr6[14]= 0x64;                addr6[15]= 0x23;
            fake_addrinfo(myindex, RT_SCOPE_LINK, &nii, addr6);
        }

        /* now make a loopback interface */
        {
            unsigned char hwaddr[6];
            hwaddr[0]=0xc0;        hwaddr[1]=0x00;
            hwaddr[2]=0x7f;        hwaddr[3]=0x00;
            hwaddr[4]=0x00;        hwaddr[5]=0x01;

            myindex = ++ifindex;
            fake_linkinfo("lo", myindex, &nii, hwaddr,
                          ARPHRD_LOOPBACK, IFF_BROADCAST|IFF_UP|IFF_RUNNING);
        }

        {
            /* now send up the IPv6 address */
            unsigned char addr6[16];
            addr6[0] = 0x00;                addr6[1] = 0x00;
            addr6[2] = 0x00;                addr6[3] = 0x00;
            addr6[4] = 0x00;                addr6[5] = 0x00;
            addr6[6] = 0x00;                addr6[7] = 0x00;
            addr6[8] = 0x00;                addr6[9] = 0x00;
            addr6[10]= 0x00;                addr6[11]= 0x00;
            addr6[12]= 0x00;                addr6[13]= 0x00;
            addr6[14]= 0x00;                addr6[15]= 0x01;
            fake_addrinfo(myindex,  RT_SCOPE_HOST, &nii, addr6);
        }

        /* this is interface wlan1, and it's not up, has no IP addresses */
        if(wlan1_is_down)
        {
            unsigned char hwaddr[6];
            hwaddr[0]=0x00;        hwaddr[1]=0x16;
            hwaddr[2]=0xf0;        hwaddr[3]=0x0d;
            hwaddr[4]=0xba;        hwaddr[5]=0xbe;

            /* now make a loopback interface */
            myindex = ++ifindex;
            fake_linkinfo("wlan1", myindex, &nii, hwaddr, ARPHRD_ETHER, IFF_BROADCAST);
        }

        /* now a bridge device that has no carrier, should get ignored */
        {
            unsigned char hwaddr[6];
            hwaddr[0]=0xc0;        hwaddr[1]=0x00;
            hwaddr[2]=0x7f;        hwaddr[3]=0xab;
            hwaddr[4]=0x00;        hwaddr[5]=0x02;

            myindex = ++ifindex;
            fake_linkinfo("virbr0", myindex, &nii, hwaddr,
                          ARPHRD_LOOPBACK, IFF_BROADCAST|IFF_UP);
            /* NOT: Not IFF_RUNNING */
        }

        {
            /* now send up the IPv6 address */
            unsigned char addr6[16];
            addr6[0] = 0x20;                addr6[1] = 0x00;
            addr6[2] = 0x0d;                addr6[3] = 0xb8;
            addr6[4] = 0x00;                addr6[5] = 0x00;
            addr6[6] = 0x00;                addr6[7] = 0x02;
            addr6[8] = 0x00;                addr6[9] = 0x03;
            addr6[10]= 0x00;                addr6[11]= 0x04;
            addr6[12]= 0x00;                addr6[13]= 0x05;
            addr6[14]= 0x00;                addr6[15]= 0x06;
            fake_addrinfo(myindex,  RT_SCOPE_HOST, &nii, addr6);
        }


}

/* used by addprefix() to change system parameters */
int pcap_network_interface::nisystem(const char *cmd)
{
        debug->log("    would invoke cmd: %s\n", cmd);
        return 0;
}

int pcap_network_interface::process_infile(const char *ifname,
                                           const char *infile,
                                           const char *outfile,
                                           rpl_debug *deb)
{
        pcap_network_interface *ndproc =
            setup_infile_outfile(ifname, infile, outfile, deb);

        ndproc->alive = true;
        ndproc->process_pcap();
        return 0;
}

void pcap_network_interface::set_pcap_out(const char *outfile,
                                          int pcap_link)
{
    pcap_t *pout = pcap_open_dead(pcap_link, 65535);
    if(!pout) {
	debug->error("can not create pcap_open_deads\n");
        exit(1);
    }

    close_pcap_files();
    this->pcap_out = pcap_dump_open(pout, outfile);

    if(!pcap_out) {
	debug->error("can not open output %s\n", outfile);
        exit(1);
    }
}

void pcap_network_interface::close_pcap_files(void)
{
    if(this->pcap_out) {
	pcap_dump_close(this->pcap_out);
    }
    this->pcap_out = NULL;
}


void pcap_network_interface::setup_infile(const char *infile)
{
    pcap_t *ppol;
    char errbuf[PCAP_ERRBUF_SIZE];

    ppol = pcap_open_offline(infile, errbuf);

    if(!ppol) {
        debug->error("can not open input %s: %s\n", infile, errbuf);
        exit(1);
    }

    if(this->pol) {
        pcap_close(this->pol);
        this->pol = NULL;
    }
    this->pol = ppol;
}

void pcap_network_interface::setup_outfile(const char *outfile)
{
    setup_outfile(outfile, this->pcap_link);
}

void pcap_network_interface::setup_outfile(const char *outfile,
                                           int pcap_link)
{
    pcap_dumper_t *out = NULL;
    if(outfile) {
        pcap_t *pout = pcap_open_dead(pcap_link, 65535);
        if(!pout) {
            debug->error("can not create pcap_open_deads\n");
            exit(1);
        }

        out = pcap_dump_open(pout, outfile);

        if(!out) {
            debug->error("can not open output %s\n", outfile);
            exit(1);
        }
    }

    this->set_link_encap(pcap_link);
    this->pcap_out = out;
}

bool pcap_network_interface::setup_lowpan(const unsigned char eui64[8], unsigned int eui64len)
{
    this->set_link_layer64(eui64, eui64len);
    debug->info("virtual interface, not assigning eui64");
    return true;
}


pcap_network_interface *pcap_network_interface::setup_infile_outfile(
    const char *ifname,
    const char *infile,
    const char *outfile,
    rpl_debug *debug)
{
        pcap_network_interface *ndproc;

        ndproc = (pcap_network_interface *)find_by_name(ifname);

        if(ndproc == NULL) {
            return NULL;
        }

        ndproc->pcap_link = DLT_EN10MB;

        if(infile) {
            ndproc->setup_infile(infile);
            if(ndproc->pol) {
                ndproc->pcap_link = pcap_datalink(ndproc->pol);
            }
        }

        ndproc->setup_outfile(outfile);
	ndproc->set_debug(debug);

        return ndproc;
}


int pcap_network_interface::process_pcap(void)
{
        if(pol==NULL) return -1;

	pcap_loop(pol, 0, sunshine_pcap_input, (u_char *)this);

	if(this->errors() > 0) {
		exit(this->errors());
	}

        return 0;
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
