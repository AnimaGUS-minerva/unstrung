PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

TESTNAME=26-builddaoack
OUTPUT=daoack

${TESTNAME}: ${TESTNAME}.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | tee ../OUTPUTS/${OUTPUT}-out.txt | diff - ${OUTPUT}-out.txt
	@tcpdump -t -n -r ../OUTPUTS/26-${OUTPUT}.pcap | tee ../OUTPUTS/${OUTPUT}.txt | diff - ${OUTPUT}.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o

update:
	cp ../OUTPUTS/${OUTPUT}-out.txt ${OUTPUT}-out.txt
	cp ../OUTPUTS/${OUTPUT}.txt ${OUTPUT}.txt