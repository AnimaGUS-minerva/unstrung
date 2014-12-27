UNSTRUNG_SRCDIR?=$(shell cd ../..; pwd)

# local install of pcap.h, or make sure you have libpcap-dev installed.
CFLAGS+=${LIBPCAPINC} -g
CXXFLAGS+=${LIBPCAPINC} -g
PROGRAM=senddio
PROGRAMDIR=/sbin
ONEFILE=senddio.cpp
OBJS=senddio.o
LIBS=${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK} ${LIBFAKEIFACE}

include ${UNSTRUNG_SRCDIR}/programs/Makefile.program
