/*
 * hexdump routine that omits lines of zeroes, except first/last
 * and it well enough commented that you won't mess it up when
 * you modify it, yet again.
 *
 * base address is pointer, and offset is into that space.
 * this is so that the offset can be printed nicely and make relative
 * sense.
 *
 * Include this where you need it.
 *
 */
#include <string.h>
#include <ctype.h>
#ifndef hexdump_printf
#define hexdump_printf printf
#endif
static void hexdump(const unsigned char *base, unsigned int offset, int len)
{
    const unsigned char *b = base+offset;
    char line[81];
    char *out; int left=sizeof(line);
    int outcount, posonthisline;
    unsigned char bb[4];             /* avoid doing byte accesses */
    int i;
    int first,last;     /* flags */
    int elided=0;

    last=0;
    first=1;
    posonthisline=0;

    for(i = 0; i < len; i++) {
        /* see if we are at the last line */
        if((len-i) <= 16) last=1;

        /* if it's the first item on the line */
        if(posonthisline == 0) {
            /* and it's not the first or last line */
            if(!first && !last) {
                int j;

                /* see if all the entries are zero */
                for(j=0; j < 4; j++) {
                    memcpy(bb, b+i+4*j, 4);
                    if(bb[0] || bb[1] || bb[2] || bb[3]) break;
                }

                /* yes, they all are */
                if(j==4) {
                    /* so just advance to next chunk,
                     * noting the i++ above. */
                    i = i+15;
                    if(!elided) {
                        hexdump_printf("...\n");
                        elided=1;
                    }
                    continue;
                }
            }

            /* print the offset */
            left=sizeof(line);
            out =line;
            memset(line, ' ', 79);
            line[80]='\0';
            outcount = snprintf(line, left, "%04x:", offset+i);
            left -= outcount;
            out  += outcount;
        }

        elided=0;
        memcpy(bb, b+i, 4);
        outcount = snprintf(out, left, " %02x %02x %02x %02x ",
                            bb[0], bb[1], bb[2], bb[3]);
        line[posonthisline+60]=(isprint(bb[0]) ? bb[0] : '.');
        line[posonthisline+61]=(isprint(bb[1]) ? bb[1] : '.');
        line[posonthisline+62]=(isprint(bb[2]) ? bb[2] : '.');
        line[posonthisline+63]=(isprint(bb[3]) ? bb[3] : '.');
        line[posonthisline+64]='\0';
        left -= outcount;
        out  += outcount;
        i    += 3;
        posonthisline += 4;

        /* see it's the last item on line */
        if(!((i + 1) % 16)) {
            first=0;
            *out=' ';
            hexdump_printf("%s\n",line);

            /* reset things */
            posonthisline=0;
            memset(line, ' ', 79);
            line[80]='\0';
        }

    }
    /* if it wasn't the last item on line */
    if(i % 16) {
        *out=' ';
        hexdump_printf("%s\n",line);
    }
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

