UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing
INC=${UNSTRUNG_SRCDIR}/include

export SENDDIO=${UNSTRUNG_SRCDIR}/programs/senddio/senddio

checkprograms::
programs::
clean::
	@rm -f core.*
	@rm -rf OUTPUT

check::
	@mkdir -p OUTPUT
	@rm -f core.*
	./senddio-test-01.sh 
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi
	./senddio-test-02.sh 
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi
	./senddio-test-03.sh 
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi
	./senddio-test-04.sh 
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi

