UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

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
