PANDORA_SRCDIR?=$(shell cd ../..; pwd)

# local install of pcap.h, or make sure you have libpcap-dev installed.
CFLAGS+=-I/sandel/include -g
CXXFLAGS+=-I/sandel/include -g
PROGRAM=senddao
PROGRAMDIR=/sbin
OBJS=senddao.o
LIBS+=${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK} ${LIBFAKEIFACE}
LDFLAGS+=${LIBPCAP}

include ${PANDORA_SRCDIR}/programs/Makefile.program
