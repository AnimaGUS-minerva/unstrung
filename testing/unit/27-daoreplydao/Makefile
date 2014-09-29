PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

TESTNAME=27-daoreplydao
OUTPUT=daoreply
OUTPUT1=node-E-dio
OUTPUT2=node-E-dio
INCLUDES=../24-daoreply/dioA.cpp ../24-daoreply/daoackA.cpp
INCLUDES+=${INC}/iface.h ${INC}/dag.h

${TESTNAME}: ${TESTNAME}.cpp ${LIBNDMGMT} ${INCLUDES}

	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | tee ../OUTPUTS/${OUTPUT}-out.txt | diff -B -w - ${OUTPUT}-out.txt
	@${TCPDUMP} -t -n -r ../OUTPUTS/27-${OUTPUT1}.pcap | tee ../OUTPUTS/${OUTPUT1}.txt | diff -B -w - ${OUTPUT1}.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o

update:
	cp ../OUTPUTS/${OUTPUT}-out.txt ${OUTPUT}-out.txt
	cp ../OUTPUTS/${OUTPUT1}.txt ${OUTPUT1}.txt

