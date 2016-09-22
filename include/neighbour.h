#ifndef _NEIGHBOUR_H_

/*
 * NOTE: the contents of this file are an interpretation of RFC6775
 *       no copyright is asserted on this file, as it transcribes
 *       a public specification.
 * It comes from https://github.com/mcr/unstrung/blob/master/include/neighbour.h
 */

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

/*
 * ICMPv6 - Neighbour Discovery --- RFC6775 extensions.
 *    /usr/include/netinet/icmp6.h usually has a base called nd_neighbor_solicit.
 */

enum ND_ND_OPTTYPE {
    ND_OPT_ARO = 33,  /* ARO */
};

/* section 4.1 - Address Registration Option */
/*
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |   Length = 2  |    Status     |   Reserved    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           Reserved            |     Registration Lifetime     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  +                            EUI-64                             +
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 */

struct nd_opt_aro {
    u_int8_t nd_aro_type;
    u_int8_t nd_aro_len;
    u_int8_t nd_aro_status;
    u_int8_t nd_aro_res1;
    u_int16_t nd_aro_res2;
    u_int16_t nd_aro_lifetime;
    u_int8_t  nd_aro_eui64[8];
} PACKED;

#define ND_ARO_DEFAULT_LIFETIME 5


#define _NEIGHBOUR_H_
#endif /*  _NEIGHBOUR_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

