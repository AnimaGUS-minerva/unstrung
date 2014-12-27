UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

TESTNAME=vf

${TESTNAME}: ${TESTNAME}.cpp ${INC}/iface.h ${INC}/dag.h base.o derived.o
	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp base.o derived.o ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | diff - expected.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o
