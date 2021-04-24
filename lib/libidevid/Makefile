# library of functions related to reading attributes out of IDevID and LDevID certificates
#
# Copyright (C) 2021 Michael Richardson <mcr@sandelman.ca>
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

LIBRARY=idevid
LIB=lib${LIBRARY}.a

ONEFILE=eui64.cpp
CFLAGS+=${WERROR}

SRCS =
CXXSRCS= eui64.cpp

CXXFLAGS+=-g ${CROSSFLAGS}
CFLAGS+=-g ${CROSSFLAGS}

OBJS=${SRCS:.c=.o} ${CXXSRCS:.cpp=.o}

include ${srcdir}../Makefile.library

