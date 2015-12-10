UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

export SENDDAO=${UNSTRUNG_OBJDIR}/programs/senddao/senddao

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

update:
	cp OUTPUTS/senddao-test-01.raw         senddao-test-01.out
	cp OUTPUTS/senddao-test-01.err         senddao-test-01.err
	cp ../OUTPUTS/senddao-test-02.raw      senddao-test-02.out
	cp ../OUTPUTS/senddao-test-02-pcap.txt senddao-test-02-pcap.txt
	cp OUTPUTS/senddao-test-03.raw         senddao-test-03.out
	cp ../OUTPUTS/senddao-test-04.raw      senddao-test-04.out
	cp ../OUTPUTS/senddao-test-04-pcap.txt senddao-test-04-pcap.txt
