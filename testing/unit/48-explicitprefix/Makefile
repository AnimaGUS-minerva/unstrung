UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

48-explicitprefix: 48-explicitprefix.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 48-explicitprefix 48-explicitprefix.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 48-explicitprefix
programs::

check::	48-explicitprefix
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13:
	./48-explicitprefix 2>&1 | tee ../OUTPUTS/48-explicitprefix-output.raw | diff -B - 48-explicitprefix-output.txt
	#@${TCPDUMP} -t -v -n -r ../OUTPUTS/17-node-A-out.pcap | tee ../OUTPUTS/17-node-A-capture.txt | diff -B - 17-node-A-capture.txt


clean::
	@rm -rf 48-explicitprefix *.o

update:
	cp ../OUTPUTS/48-explicitprefix-output.raw  48-explicitprefix-output.txt
	#cp ../OUTPUTS/17-node-A-capture.txt  17-node-A-capture.txt

pcapupdate:
	cp ../OUTPUTS/24-node-E-out.pcap     ../INPUTS/daoE.pcap

