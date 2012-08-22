UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LDFLAGS+=${LIBNDMGMT} ${LIBNETLINK} ${LIBOSW} ${LIBPCAP}

event-01: event-01.cpp ${INC}/event.h 
	${CXX} ${CXXFLAGS} -o event-01 event-01.cpp ${LDFLAGS}

check::	event-01
	./event-01

checkprograms:: event-01
programs::

clean::
	@rm -rf event-01 *.o
