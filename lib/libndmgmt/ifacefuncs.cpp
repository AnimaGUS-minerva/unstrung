/*
 * Copyright (C) 2009-2016 Michael Richardson <mcr@sandelman.ca>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 */

/*
 * this is a set of utility functions which might have wider use.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "rpl.h"
#include "hexdump.c"

}

#include "iface.h"

int network_interface::parse_eui2bin(unsigned char *dst, size_t dstlen,
                                      const char *srcbuf)
{
    const char *p;
    bool highnibble = true;
    unsigned int nibble = 0;
    unsigned int upper4 = 0;
    int  retlen = 0;

    if(dstlen > 0 && dst) {
        *dst = '\0';
    }

    for(p = srcbuf; dstlen > 0 && *p != '\0'; p++) {
        if(*p >= '0' && *p <= '9') {
            nibble = *p - '0';
        } else if (*p >= 'A' && *p <= 'F') {
            nibble = *p - 'A' + 10;
        } else if (*p >= 'a' && *p <= 'f') {
            nibble = *p - 'a' + 10;
        } else if (*p == '-' || *p == ':') {
            /* finish byte, advance */
            if(highnibble) {
                /* no digits, or two digits, nothing to do */
                continue;
            } else {
                /* must be one digit, move it to low nibble, reset highnibble */
                nibble = upper4 >> 4;
                upper4 = 0;
            }
        } else {
            /* invalid, return false */
            return -1;
        }

        if(highnibble) {
            upper4 = nibble << 4;
            highnibble = false;
        } else {
            *dst = upper4 | nibble;
            highnibble = true;
            upper4 = 0;
            dst++;
            dstlen--;
            retlen++;
        }
    }

    return retlen;
}

bool network_interface::fmt_eui(char *txtbuf, size_t txtlen,
                                const unsigned char *srceui, size_t srclen)
{
    const char *sep = "";
    while(srclen > 0 && txtlen > 3) {
        unsigned int used = snprintf(txtbuf, txtlen, "%s%02x", sep, *srceui);
        if(used > 0) {
            txtbuf += used;
            txtlen -= used;
        } else {
            return false;
        }
        sep="-";
        srceui++;
        srclen--;
    }
    if(srclen != 0) return false;
    return true;
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
