PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

TESTNAME=vf

${TESTNAME}: ${TESTNAME}.cpp ${INC}/iface.h ${INC}/dag.h base.o derived.o
	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp base.o derived.o ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | diff - expected.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o
