UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing
INC=${UNSTRUNG_SRCDIR}/include

export SENDDIO=${UNSTRUNG_OBJDIR}/programs/senddio/senddio

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
	./senddio-test-05.sh
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi

update:
	cp OUTPUT/senddio-test-01.err senddio-test-01.err
	cp OUTPUT/senddio-test-02.raw senddio-test-02.out
	cp ../OUTPUTS/senddio-test-03.raw      senddio-test-03.out
	cp ../OUTPUTS/senddio-test-03-pcap.raw senddio-test-03-pcap.txt
	cp ../OUTPUTS/senddio-test-04a.raw     senddio-test-04.out
	cp ../OUTPUTS/senddio-test-05.raw     senddio-test-05.out
	cp ../OUTPUTS/senddio-test-05-pcap.raw     senddio-test-05-pcap.txt


