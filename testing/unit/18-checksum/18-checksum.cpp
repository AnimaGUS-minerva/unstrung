#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

class csum_network_interface : public network_interface {
public:
    void test1(void);
    void test2(void);
    void test3(void);
    void test4(void);
};

void csum_network_interface::test1(void)
{
    unsigned char ex1[]={
        0x00, 0x01,
        0xf2, 0x03,
        0xf4, 0xf5,
        0xf6, 0xf7
    };


    unsigned short result = csum_partial(ex1, 8, 0);
    result = (~result & 0xffff);
    printf("result=%04x\n",result);
    assert(result == htons(0x220d));
}

void csum_network_interface::test2(void)
{
    unsigned char ex1[]={
        0x00, 0x01,
        0xf2, 0x03,
    };
    unsigned char ex2[]={
        0xf4, 0xf5,
        0xf6, 0xf7
    };


    unsigned short result = csum_partial(ex1, 4, 0);
    result = csum_partial(ex2, 4, result);
    result = (~result & 0xffff);
    printf("result=%04x\n",result);
    assert(result == htons(0x220d));
}

/* validate calculation of pseudo-header */
void csum_network_interface::test3(void)
{
    struct ip6_hdr v6;
    unsigned char src[]={
        0xfe, 0x80, 0,    0,    0,    0,    0,    0,
        0x2,  0x16, 0x3e, 0xff, 0xfe, 0x11, 0x34, 0x24
    };
    unsigned char dst[]={
        0xff, 0x02, 0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    1
    };

    memcpy(&v6.ip6_src.s6_addr, src, 16);
    memcpy(&v6.ip6_dst.s6_addr, dst, 16);
    unsigned int icmp_len = 24;


    // 0000: fe 80 00 00  00 00 00 00  02 16 3e ff  fe 11 34 24
    // 0010: ff 02 00 00  00 00 00 00  00 00 00 00  00 00 00 01
    // 0020: 00 00 00 18  00 00 00 3a

    unsigned short icmp6sum = csum_ipv6_magic(&v6.ip6_src,
                                              &v6.ip6_dst,
                                              icmp_len, IPPROTO_ICMPV6,
                                              0);
    icmp6sum = (~icmp6sum & 0xffff);

    unsigned char all[]={
        0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x16, 0x3e, 0xff, 0xfe, 0x11, 0x34, 0x24,
        0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x3a
    };
    unsigned short result2 = csum_partial(all, 5*8, 0);
    result2 = (~result2 & 0xffff);

    printf("result2=%04x\n",result2);
    assert(result2 == htons(0x8edd));
    assert(icmp6sum == result2);
}

/* validate checksum of entire packet */
void csum_network_interface::test4(void)
{
    /* we produced */
    unsigned char pkt[]={
        /* pseudo header */
        0xfe,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
        0x02,0x16,0x3e,0xff,0xfe,0x11,0x34,0x24,
        0xff,0x02,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x3a,
        /* icmp payload */
        0x9b, 0x02, 0x39, 0x8d, 0x01, 0x40, 0x00, 0x01,
        0x70, 0x61, 0x6e, 0x64, 0x6f, 0x72, 0x61, 0x20,
        0x69, 0x73, 0x20, 0x66, 0x75, 0x6e, 0x0a, 0x6c
    };

    unsigned short result = csum_partial(pkt, sizeof(pkt), 0);
    result = (~result & 0xffff);
    assert(result == 0);
}


int main(int argc, char *argv[])
{
    csum_network_interface n1;

    n1.test1();
    n1.test2();
    n1.test3();
    n1.test4();

    exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
