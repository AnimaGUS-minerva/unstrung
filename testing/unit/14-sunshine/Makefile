UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)
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
	@(echo file ${SUNSHINE}; echo set args "--help") >.gdbinit
	${SUNSHINE} --help 2>&1 | tee ../OUTPUTS/sunshine-help.raw | sed -f ${PATHNAMES} | diff - sunshine-help.txt
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi

update:
	sed -f ${PATHNAMES} ../OUTPUTS/sunshine-help.raw   >sunshine-help.txt

