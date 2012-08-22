UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LIBNDMGMT=${UNSTRUNG_OBJDIR}/lib/libndmgmt/libndmgmt.a
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt ${LIBOSW} 
LDFLAGS+=${LIBNETLINK} ${LIBFAKEIFACE} ${LIBPCAP}

dioopt-01: dioopt-01.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o dioopt-01 dioopt-01.cpp ${LDFLAGS}

check::	dioopt-01
	./dioopt-01

checkprograms:: dioopt-01
programs::

clean::
	@rm -rf dioopt-01 *.o
