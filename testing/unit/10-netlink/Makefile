UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LIBNDMGMT=${UNSTRUNG_OBJDIR}/lib/libndmgmt/libndmgmt.a
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt ${LIBOSW} ${LIBNETLINK} ${LIBPCAP}

findeui: findeui.cpp ${LIBNETLINK} ${LIBOSW}
	${CXX} ${CXXFLAGS} -o findeui findeui.cpp ${LDFLAGS}

check::	findeui
	./findeui 2>&1 | diff - marajade.output.txt

checkprograms:: findeui
programs::

clean::
	@rm -rf findeui *.o
