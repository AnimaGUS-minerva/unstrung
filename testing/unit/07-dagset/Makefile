UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

TESTNAME=dagset-01

${TESTNAME}: ${TESTNAME}.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME}

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o
