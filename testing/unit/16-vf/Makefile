UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LDFLAGS+=${LIBFAKEIFACE}
LDFLAGS+=${LIBNDMGMT}
LDFLAGS+=${LIBOSW}
LDFLAGS+=${LIBNETLINK}
#LDFLAGS+=/sandel/lib/libpcap.a
LDFLAGS+=/corp/projects/Misc/tcpdump/libpcap/libpcap.a

TESTNAME=vf

${TESTNAME}: ${TESTNAME}.cpp ${INC}/iface.h ${INC}/dag.h base.o derived.o
	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp base.o derived.o ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | diff - expected.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o
