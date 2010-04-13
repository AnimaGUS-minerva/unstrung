UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LIBNDMGMT=${UNSTRUNG_OBJDIR}/lib/libndmgmt/libndmgmt.a
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt ${LIBOSW}
#LDFLAGS+=/sandel/lib/libpcap.a
LDFLAGS+=/corp/projects/Misc/tcpdump/libpcap/libpcap.a

dag-02: dag-02.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o dag-02 dag-02.cpp ${LDFLAGS}

check::	dag-02
	./dag-02

checkprograms:: dag-02
programs::

clean::
	@rm -rf dag-02 *.o
