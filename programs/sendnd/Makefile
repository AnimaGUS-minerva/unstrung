UNSTRUNG_SRCDIR?=$(shell cd ../..; pwd)

# local install of pcap.h, or make sure you have libpcap-dev installed.
CFLAGS+=${LIBPCAPINC} -g
CXXFLAGS+=${LIBPCAPINC} -g
PROGRAM=sendnd
PROGRAMDIR=/sbin
ONEFILE=sendnd.cpp
OBJS=sendnd.o
LIBS+=${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK} ${LIBFAKEIFACE} ${LIBGRASP} ${LIBCBOR} ${LIBMBEDTLS}
LDFLAGS+=${LIBPCAP}

include ${UNSTRUNG_SRCDIR}/programs/Makefile.program
