UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LIBS=${LIBFAKEIFACE} ${LIBNDMGMT} ${LIBOSW} ${LIBNETLINK} ${LIBPCAP}
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt  ${LIBS}

selfprefix-13: selfprefix-13.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o selfprefix-13 selfprefix-13.cpp ${LDFLAGS}

checkprograms:: selfprefix-13
programs::

check::	selfprefix-13
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13: 
	./selfprefix-13 2>&1 | tee ../OUTPUTS/selfprefix-13-output.raw | diff -B - selfprefix-13-output.txt

clean::
	@rm -rf selfprefix-13 *.o
