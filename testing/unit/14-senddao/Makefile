UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

export SENDDAO=${UNSTRUNG_SRCDIR}/programs/senddao/senddao

checkprograms::
programs::
clean::
	@rm -f core.*
	@rm -rf OUTPUT

check::
	@rm -f core.*
	./senddao-test-01.sh 
	./senddao-test-02.sh 
	./senddao-test-03.sh 
	./senddao-test-04.sh 
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi

