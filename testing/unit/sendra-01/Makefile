UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

check:
	mkdir -p OUTPUT
	SENDRA=${UNSTRUNG_SRCDIR}/programs/sendra/sendra ./sendra-test-01.sh 
	SENDRA=${UNSTRUNG_SRCDIR}/programs/sendra/sendra ./sendra-test-02.sh 
