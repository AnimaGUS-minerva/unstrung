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
#ifndef hexdump_printf
#define hexdump_printf printf
#endif
void hexdump(const unsigned char *base, unsigned int offset, int len)
{
	const unsigned char *b = base+offset;
	unsigned char bb[4];             /* avoid doing byte accesses */
	int i;
	int first,last;     /* flags */
	int elided=0;

	last=0;
	first=1;
  
	for(i = 0; i < len; i++) {
		/* see if we are at the last line */
		if((len-i) <= 16) last=1;

		/* if it's the first item on the line */
		if((i % 16) == 0) {
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
						printf("...\n");
						elided=1;
					}
					continue;
				}
			}
			
			/* print the offset */
			hexdump_printf("%04x:", offset+i);
		}

		elided=0;
		memcpy(bb, b+i, 4);
		hexdump_printf(" %02x %02x %02x %02x ",
			       bb[0], bb[1], bb[2], bb[3]);
		i+=3;

		/* see it's the last item on line */
		if(!((i + 1) % 16)) {
			first=0;
			hexdump_printf("\n");
		}

	}
	/* if it wasn't the last item on line */
	if(i % 16) {
		hexdump_printf("\n");
	}
}

