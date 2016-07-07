UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing
INC=${UNSTRUNG_SRCDIR}/include

export PECK=${UNSTRUNG_OBJDIR}/programs/peck/peck

checkprograms::
programs::
clean::
	@rm -f core.*
	@rm -rf OUTPUT

check::
	@mkdir -p OUTPUT
	@rm -f core.*
	./peck-test-01.sh
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi

update:
	cp OUTPUT/peck-test-01.raw peck-test-01.out


