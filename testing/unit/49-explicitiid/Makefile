UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

49-explicitiid: 49-explicitiid.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 49-explicitiid 49-explicitiid.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 49-explicitiid
programs::

check::	49-explicitiid
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13:
	./49-explicitiid 2>&1 | tee ../OUTPUTS/49-explicitiid-output.raw | diff -B - 49-explicitiid-output.txt
	#@${TCPDUMP} -t -v -n -r ../OUTPUTS/17-node-A-out.pcap | tee ../OUTPUTS/17-node-A-capture.txt | diff -B - 17-node-A-capture.txt


clean::
	@rm -rf 49-explicitiid *.o

update:
	cp ../OUTPUTS/49-explicitiid-output.raw  49-explicitiid-output.txt
	#cp ../OUTPUTS/17-node-A-capture.txt  17-node-A-capture.txt

pcapupdate:
	cp ../OUTPUTS/24-node-E-out.pcap     ../INPUTS/daoE.pcap

