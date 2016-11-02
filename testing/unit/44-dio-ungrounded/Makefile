UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

TESTNAME=44-dio-ungrounded
OUTPUT=daoreply
OUTPUT1=node-E-dio
OUTPUT2=node-E-dio
INCLUDES=../24-daoreply/dioA.cpp ../24-daoreply/daoackA.cpp
INCLUDES+=${INC}/iface.h ${INC}/dag.h

${TESTNAME}: ${TESTNAME}.cpp ${LIBNDMGMT} ${INCLUDES}

	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | tee ../OUTPUTS/${OUTPUT}-out.txt | diff -B -w - ${OUTPUT}-out.txt
	@${TCPDUMP} -t -n -r ../OUTPUTS/44-${OUTPUT1}.pcap | tee ../OUTPUTS/${OUTPUT1}.txt | diff -B -w - ${OUTPUT1}.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o

update:
	cp ../OUTPUTS/${OUTPUT}-out.txt ${OUTPUT}-out.txt
	cp ../OUTPUTS/${OUTPUT1}.txt ${OUTPUT1}.txt

pcapupdate:
	: cp ../OUTPUTS/39-dao-J.pcap  ../INPUTS/44-daoJ-inst42.pcap
	cp ../OUTPUTS/senddio-test-05.pcap ../INPUTS/dio-A-661e-ungrounded.pcap
