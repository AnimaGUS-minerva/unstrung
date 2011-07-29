UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LIBS=${LIBFAKEIFACE} ${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK} ${LIBPCAP}
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt ${LIBS}

dag-02: dag-02.cpp ${INC}/iface.h ${INC}/dag.h ${LIBS}
	${CXX} ${CXXFLAGS} -o dag-02 dag-02.cpp ${LDFLAGS}

check::	dag-02
	./dag-02

checkprograms:: dag-02
programs::

clean::
	@rm -rf dag-02 *.o
