UNSTRUNG_SRCDIR?=$(shell cd ../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc

# local install of pcap.h, or make sure you have libpcap-dev installed.
CFLAGS+=-I/sandel/include -g
PROGRAM=peck
PROGRAMDIR=/sbin
OBJS=peck.o

include ${UNSTRUNG_SRCDIR}/programs/Makefile.program
