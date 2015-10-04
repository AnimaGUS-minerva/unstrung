UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

30-ackdao: 30-ackdao.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 30-ackdao 30-ackdao.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 30-ackdao
programs::

check::	30-ackdao
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13:
	./30-ackdao ../INPUTS/29-dao-nopio.pcap /dev/null 2>&1 | tee ../OUTPUTS/30-ackdao-output.raw | diff -B - 30-ackdao-output.txt
	@${TCPDUMP} -t -v -n -r ../OUTPUTS/30-node-A-out.pcap | tee ../OUTPUTS/30-node-A-capture.txt | diff -B - 30-node-A-capture.txt


clean::
	@rm -rf 30-ackdao *.o

update:
	cp ../OUTPUTS/29-node-E-out.pcap ../INPUTS/29-dao-nopio.pcap
	cp ../OUTPUTS/30-ackdao-output.raw  30-ackdao-output.txt
	cp ../OUTPUTS/30-node-A-capture.txt 30-node-A-capture.txt

