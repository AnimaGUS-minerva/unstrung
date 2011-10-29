#include <stdio.h>
#include <stdlib.h>
#include "iface.h"

#include "fakeiface.h"

class csum_network_interface : public network_interface {
public:
    void test1(void);
    void test2(void);
    void test3(void);
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
    printf("result=%04x\n",result);
    assert(result == 0xddf2);
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
    printf("result=%04x\n",result);
    assert(result == 0xddf2);
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

    unsigned char all[]={
        0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x16, 0x3e, 0xff, 0xfe, 0x11, 0x34, 0x24,
        0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x3a
    };
    unsigned short result2 = csum_partial(all, 5*8, 0);

    assert(result2 == 0x7122);
    assert(icmp6sum == result2);
}


int main(int argc, char *argv[])
{
    csum_network_interface n1;

    n1.test1();
    n1.test2();
    n1.test3();

    exit(0);
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
