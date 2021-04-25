UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

TESTNAME=46-method-varargs
OUTPUT=method-varargs
INCLUDES=

${TESTNAME}: ${TESTNAME}.cpp ${LIBNDMGMT} ${INCLUDES}
	${CXX} ${CXXFLAGS} -o ${TESTNAME} ${TESTNAME}.cpp ${LDFLAGS}

check::	${TESTNAME}
	./${TESTNAME} | tee ../OUTPUTS/${OUTPUT}-out.txt | diff -B -w - ${OUTPUT}-out.txt

checkprograms:: ${TESTNAME}
programs::

clean::
	@rm -rf ${TESTNAME} *.o

update:
	cp ../OUTPUTS/${OUTPUT}-out.txt ${OUTPUT}-out.txt

pcapupdate:
	@true


