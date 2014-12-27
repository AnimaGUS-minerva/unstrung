UNSTRUNG_SRCDIR?=$(shell cd ../..; pwd)

# local install of pcap.h, or make sure you have libpcap-dev installed.
CFLAGS+=${LIBPCAPINC} -g
CXXFLAGS+=${LIBPCAPINC} -g
PROGRAM=senddao
PROGRAMDIR=/sbin
ONEFILE=senddao.cpp
OBJS=senddao.o
LIBS+=${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK} ${LIBFAKEIFACE}
LDFLAGS+=${LIBPCAP}

include ${UNSTRUNG_SRCDIR}/programs/Makefile.program
