/*
 * internal definitions for use within the library; do not export!
 * Copyright (C) 1998, 1999  Henry Spencer.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/lgpl.txt>.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * RCSID $Id: internal.h,v 1.12 2004/04/11 17:08:24 mcr Exp $
 */

#ifndef _OSWLIBS_H_

#ifndef ABITS
#define	ABITS	32	/* bits in an IPv4 address */
#endif

/* case-independent ASCII character equality comparison */
#define	CIEQ(c1, c2)	( ((c1)&~040) == ((c2)&~040) )

/*
 * Headers, greatly complicated by stupid and unnecessary inconsistencies
 * between the user environment and the kernel environment.  These are done
 * here so that this mess need exist in only one place.
 *
 * It may seem like a -I or two could avoid most of this, but on closer
 * inspection it is not quite that easy.
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* You'd think this would be okay in the kernel too -- it's just a */
/* bunch of constants -- but no, in RH5.1 it screws up other things. */
/* (Credit:  Mike Warfield tracked this problem down.  Thanks Mike!) */
/* Fortunately, we don't need it in the kernel subset of the library. */
#include <limits.h>

/* header files for things that should never be called in kernel */
#include <netdb.h>

/* memory allocation, currently user-only, macro-ized just in case */
#include <stdlib.h>
#define	MALLOC(n)	malloc(n)
#define	FREE(p)		free(p)


typedef const char *err_t;	/* error message, or NULL for success */
typedef struct {
	union {
		struct sockaddr_in v4;
		struct sockaddr_in6 v6;
	} u;
} ip_address;

typedef struct {
	ip_address addr;
	int maskbits;
} ip_subnet;

extern err_t ttosubnet(const char *src, size_t srclen, int af, ip_subnet *dst);
/* looks up names in DNS */
extern err_t ttoaddr(const char *src, size_t srclen, int af, ip_address *dst);
extern err_t tnatoaddr(const char *src, size_t srclen, int af, ip_address *dst);

extern void setportof(int port, ip_address *dst);
extern err_t ttoul(const char *src, size_t srclen, int format, unsigned long *dst);
extern int masktocount(const ip_address *src);
extern err_t initsubnet(const ip_address *addr, int maskbits, int clash, ip_subnet *dst);
extern size_t addrbytesptr(const ip_address *src, const unsigned char **dst);
extern size_t addrbytesptr_write(ip_address *src, unsigned char **dst);
extern size_t addrbytesof(const ip_address *src, unsigned char *dst, size_t dstlen);
extern int portof(const ip_address *src);
extern void setportof(int port, ip_address *dst);
extern err_t initaddr(const unsigned char *src, size_t srclen, int af, ip_address *dst);
extern err_t unspecaddr(int af, ip_address *dst);
extern int addrtypeof(const ip_address *src);
extern size_t addrlenof(const ip_address *src);
extern err_t addrtosubnet(const ip_address *addr, ip_subnet *dst);
extern struct sockaddr *sockaddrof(ip_address *src);
extern size_t sockaddrlenof(const ip_address *src);
extern int subnettypeof(const ip_subnet *src);
extern void networkof(const ip_subnet *src, ip_address *dst);
extern void maskof(const ip_subnet *src, ip_address *dst);
extern err_t add_port(int af, ip_address *addr, unsigned short port);
extern err_t anyaddr(int af, ip_address *dst);
extern err_t loopbackaddr(int af, ip_address *dst);
extern int isanyaddr(const ip_address *src);
extern int isunspecaddr(const ip_address *src);
extern int isloopbackaddr(const ip_address *src);

int isvalidsubnet(const ip_subnet *a);
size_t addrtot(const ip_address *src, int format, char *buf, size_t buflen);
#define	ADDRTOT_BUF	(32*2 + 3 + 1 + 3 + 1 + 1)

/* format: 0=default, 'Q'=a.b.c.d, 'r'=d.c.b.a, */
size_t subnettot(const ip_subnet *src, int format, char *buf, size_t buflen);
#define	SUBNETTOT_BUF	(ADDRTOT_BUF + 1 + 3)

size_t ultot(unsigned long src, int format, char *buf, size_t buflen);
#define	ULTOT_BUF	(22+1)	/* holds 64 bits in octal */

size_t subnetporttot(const ip_subnet *sub,
                     int format, char *dst,
                     size_t dstlen);


#define _OSWLIBS_H_
#endif /* _OSWLIBS_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

