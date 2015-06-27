# Copyright (C) 2002-2015  Michael Richardson
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

UNSTRUNG_SRCDIR?=../../..
OBJS = nethub.o cmdmode.o port.o hash.o tuntap.o
OBJS+=getopt_long.o

NOBJS = uml_netjig.o $(OBJS)
SOBJS = uml_switch.o $(OBJS)

OPTIONS=-DARP_PROCESS -DTUNTAP

# To use the netdissect option, you have to have at least version 4.3.0
# of tcpdump source code installed.
#
# If you want to play with it, you'll need libnetdissect.a. You can get this
# by checking code out of www.tcpdump.org.
#
NETDISSECTH?=-DNETDISSECT -I${UNSTRUNG_SRCDIR}/../tcpdump-4.6.0
NETDISSECTH+=-D__ATTRIBUTE___NORETURN_OK_FOR_FUNCTION_POINTERS -D__ATTRIBUTE___FORMAT_OK_FOR_FUNCTION_POINTERS

# this is set in Makefile.inc and probably overridden in Makefile.local
#NETDISSECTLIB?=-L${UNSTRUNG_SRCDIR}/../tcpdump-4.6.0-host -lnetdissect

# to decode ESP packets (with netdissect), you need crypto, which you can
# get from ssl.
OPENSSL=-L/usr/local/ssl/lib -lcrypto

# To read/write files, you need to have libpcap (-devel if on RH) installed.
PCAPH=-I/usr/include/pcap
#PCAPH=-I/home/mcr/src/libpcap

CFLAGS = -g -Wall ${PCAPH} ${NETDISSECTH} ${OPTIONS}
LIBS = ${NETDISSECTLIB} ${LIBPCAP_HOST} ${OPENSSL}


include $(UNSTRUNG_SRCDIR)/Makefile.inc

def:
	@echo "Please read doc/intro.html or INSTALL before running make"
	@false

checkprograms:: uml_netjig uml_switch

uml_netjig : $(NOBJS)
	$(CC) $(CFLAGS) -o uml_netjig $(NOBJS) $(LIBS)

uml_switch : $(SOBJS)
	$(CC) $(CFLAGS) -o uml_switch $(SOBJS) $(LIBS)

clean::
	rm -f uml_netjig uml_switch $(SOBJS) $(NOBJS) *~

install_check: uml_netjig
	@install -d $(LIBEXECDIR)
	@install -s uml_netjig $(LIBEXECDIR)

install_file_list:
	@true

install programs check::
	@true

depend:
	makedepend -- ${CFLAGS} -- ${NOBJS:.o=.c} uml_netjig.c uml_switch.c

# DO NOT DELETE - these were hand edited.

nethub.o: port.h hash.h nethub.h netjig.h nethub.c
cmdmode.o: nethub.h netjig.h cmdmode.c
port.o: hash.h port.h nethub.h port.c
hash.o: switch.h hash.h hash.c
tuntap.o: port.h tuntap.c
uml_netjig.o: port.h hash.h netjig.h nethub.h uml_netjig.c
uml_switch.o: port.h hash.h netjig.h nethub.h uml_netjig.c
