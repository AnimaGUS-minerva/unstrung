# Derived from Openswan library
#
# Copyright (C) 1998-2001  Henry Spencer.
# Copyright (C) 2003, 2004 Xelerance Corporation
# Copyright (C) 2009 Michael Richardson <mcr@sandelman.ca>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#

UNSTRUNG_SRCDIR?=$(shell cd ../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.ver
include ${UNSTRUNG_SRCDIR}/Makefile.inc

LIBRARY=osw
LIB=lib${LIBRARY}.a
MANDIR=$(MANTREE)/man3

ONEFILE=anyaddr.c
# turn off -Wcast-qual
WCASTQUAL= -Wno-error=cast-qual
CFLAGS+=${WERROR}

SRCS = ttosubnet.c ttoaddr.c addrtypeof.c subnettypeof.c
SRCS+= ttoul.c initsubnet.c portof.c initaddr.c anyaddr.c
SRCS+= addrtot.c ultot.c subnettot.c sameaddr.c
CXXSRCS=

#enable to get lots more debugging about semantics.
#CFLAGS=-DPARSER_TYPE_DEBUG
CXXFLAGS+=-g ${CROSSFLAGS}
CFLAGS+=-g ${CROSSFLAGS}

# must turn this off due to sameaddr.c
CFLAGS+= -Wno-error=cast-qual

OBJS=${SRCS:.c=.o} ${CXXSRCS:.cpp=.o}

include ${srcdir}../Makefile.library

