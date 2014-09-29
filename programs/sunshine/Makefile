# Makefile for the UNSTRUNG sunshine daemon
# Copyright (C) 2009-2013 Michael Richardson <mcr@sandelman.ca>
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


PANDORA_SRCDIR?=$(shell cd ../..; pwd)
srcdir?=${PANDORA_SRCDIR}/programs/sunshine/

PROGRAM=sunshine
OBJS=${SRCS:.c=.o}
SRCS=main.cpp sendnd.cpp
LIBS+=${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK}
CFLAGS+=${LIBPCAPINC}
CXXFLAGS+=${LIBPCAPINC}
CXXFLAGS+=-g -DBUILDNUMBER=\"${BUILDNUMBER}\"
LDFLAGS=-g

include ${PANDORA_SRCDIR}/programs/Makefile.program

