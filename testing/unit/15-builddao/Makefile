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

TESTNAME=builddao-01

${TESTNAME}: ${TESTNAME}.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | tee ../OUTPUTS/15-dao-out.txt | diff - dao-out.txt
	@tcpdump -t -n -r ../OUTPUTS/15-dao.pcap | tee ../OUTPUTS/15-dao.txt | diff - dao.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o
