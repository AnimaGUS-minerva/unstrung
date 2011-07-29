UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LIBNDMGMT=${UNSTRUNG_OBJDIR}/lib/libndmgmt/libndmgmt.a
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt ${LIBOSW} ${LIBNETLINK}
#LDFLAGS+=/sandel/lib/libpcap.a
LDFLAGS+=/corp/projects/Misc/tcpdump/libpcap/libpcap.a ${LIBFAKEIFACE}

dag-03: dag-03.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o dag-03 dag-03.cpp ${LDFLAGS}

check::	dag-03
	./dag-03

checkprograms:: dag-03
programs::

clean::
	@rm -rf dag-03 *.o
