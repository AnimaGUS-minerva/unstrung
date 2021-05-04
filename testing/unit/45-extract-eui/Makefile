UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

TESTNAME=extract-eui

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

EXTRALIBS=${LIBIDEVID} ${MBEDTLSLIB}/libmbedx509.a

${TESTNAME}: ${TESTNAME}.cpp ${INC}/iface.h ${LIBS} ${LIBIDEVID}
	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME}

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o
