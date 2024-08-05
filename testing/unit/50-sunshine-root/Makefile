UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)
ARGS=-I 1 --dagid C0:FFEE::C0:FFEE --ldevid hostcert.pem --stderr --verbose -W 100000 -h
PATHNAMES=../14-sunshine/pathnames.sed

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

export SUNSHINE=${UNSTRUNG_OBJDIR}/programs/sunshine/sunshine

checkprograms::
programs::
clean::
	@rm -f core.*
	@rm -rf OUTPUTS

check::
	@(echo file ${SUNSHINE}; echo set args $ARGS) >.gdbinit
	${SUNSHINE} ${ARGS} 2>&1 | tee ../OUTPUTS/sunshine-root.raw | sed -f ${PATHNAMES} | diff - sunshine-root.txt
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi

update:
	sed -f ${PATHNAMES} ../OUTPUTS/sunshine-root.raw   >sunshine-root.txt

