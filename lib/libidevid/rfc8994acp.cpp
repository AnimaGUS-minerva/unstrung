/*
 * Copyright (C) 2009-2021 Michael Richardson <mcr@sandelman.ca>
 *
 * SEE ../../LICENSE
 *
 * This implements parsing of RFC8994 otherName (and rfc822Name hack),
 *
 *
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/ip6.h>
}

#include "devid.h"

bool device_identity::parse_rfc8994string( const char *prefix,
                                           size_t prefix_len,
                                           ip_subnet *sn)
{
    /* duplicate the string so that it can be taken apart */
    char *prefix2 = (char *)alloca(prefix_len);
    memcpy(prefix2, prefix, prefix_len);

    char *at   = strchr(prefix2, '@');
    if(at) {
        *at = '\0';
        at++;
    }

    char *plus = strchr(prefix2, '+');
    if(plus == NULL) {
        return false;
    }
    *plus = '\0';
    plus++;

    char *plus2 = strchr(plus, '+');
    if(plus2) {
        *plus2 = '\0';
        plus2++;
    }

    if(strcmp("rfc8994", prefix2) != 0 &&
       strcmp("rfcSELF", prefix2) != 0) {
        return false;
    }

    /* eat all the hex digits between plus and plus2 */
    for(int i=0; i < 16; i++) {
        sn->addr.u.v6.sin6_addr.s6_addr[i] = 0;
        if(plus[0] != '\0' && plus[1] != '\0') {
            if(sscanf(plus, "%02x", &sn->addr.u.v6.sin6_addr.s6_addr[i]) != 1) {
                return false;
            }
            plus += 2;
        }
    }
    sn->maskbits = 128;
    sn->addr.u.v6.sin6_family = AF_INET6;

    return true;
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

