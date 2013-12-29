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

UNSTRUNG_SRCDIR?=$(shell cd ../..; pwd)
srcdir?=${UNSTRUNG_SRCDIR}/programs/peck/

PROGRAM=peck
# local install of pcap.h, or make sure you have libpcap-dev installed.
CFLAGS+=-I/sandel/include -g
PROGRAMDIR=/sbin
OBJS=peck.o

include ${UNSTRUNG_SRCDIR}/programs/Makefile.program
