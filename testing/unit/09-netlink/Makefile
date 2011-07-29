UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LDFLAGS+=${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK}
#LDFLAGS+=/sandel/lib/libpcap.a
LDFLAGS+=/corp/projects/Misc/tcpdump/libpcap/libpcap.a

dumpif: dumpif.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o dumpif dumpif.cpp ${LDFLAGS}

check::	dumpif
	./dumpif | grep "lo:"

checkprograms:: dumpif
programs::

clean::
	@rm -rf dumpif *.o
