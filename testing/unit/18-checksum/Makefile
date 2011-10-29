UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LIBS=${LIBFAKEIFACE} ${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK} ${LIBPCAP}
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt  ${LIBS}

18-checksum: 18-checksum.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 18-checksum 18-checksum.cpp ${LDFLAGS}

checkprograms:: 18-checksum
programs::

check::	18-checksum
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13: 
	./18-checksum 
	@echo SUCCEED

clean::
	@rm -rf 18-checksum *.o
