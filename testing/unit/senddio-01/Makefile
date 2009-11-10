UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

check:
	mkdir -p OUTPUT
	SENDDIO=${UNSTRUNG_SRCDIR}/programs/senddio/senddio ./senddio-test-01.sh 
	SENDDIO=${UNSTRUNG_SRCDIR}/programs/senddio/senddio ./senddio-test-02.sh 
