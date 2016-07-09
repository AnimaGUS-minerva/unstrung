UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

TESTNAME=39-daoJ
OUTPUT=node-J-dao
OUTPUT1=39-dao-J
INCLUDES=../24-daoreply/dioA.cpp ../24-daoreply/daoackA.cpp
INCLUDES+=${INC}/iface.h ${INC}/dag.h

${TESTNAME}: ${TESTNAME}.cpp ${LIBNDMGMT} ${INCLUDES}

	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | tee ../OUTPUTS/${OUTPUT}-out.txt | diff -B -w - ${OUTPUT}-out.txt
	@${TCPDUMP} -t -n -r ../OUTPUTS/${OUTPUT1}.pcap | tee ../OUTPUTS/${OUTPUT1}.txt | diff -B -w - ${OUTPUT1}.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o

update:
	cp ../OUTPUTS/${OUTPUT}-out.txt ${OUTPUT}-out.txt
	cp ../OUTPUTS/${OUTPUT1}.txt ${OUTPUT1}.txt

pcapupdate:
	cp ../OUTPUTS/24-node-E-dio.pcap ../INPUTS/24-node-E-dio.pcap

