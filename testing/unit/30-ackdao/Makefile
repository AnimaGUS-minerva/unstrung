UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

all:  30-ackdao .gdbinit

30-ackdao: 30-ackdao.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 30-ackdao 30-ackdao.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 30-ackdao
programs::

.gdbinit: Makefile
	@echo file ./30-ackdao >.gdbinit
	@echo set args "../INPUTS/29-dao-nopio.pcap ../INPUTS/31-dao-extra.pcap ../INPUTS/32-dao-onlyif.pcap" >>.gdbinit

check::	30-ackdao
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13:

	./30-ackdao ../INPUTS/29-dao-nopio.pcap ../INPUTS/31-dao-extra.pcap ../INPUTS/32-dao-onlyif.pcap 2>&1 | tee ../OUTPUTS/30-ackdao-output.raw | diff -B - 30-ackdao-output.txt
	@${TCPDUMP} -t -v -n -r ../OUTPUTS/30-node-A-out.pcap | tee ../OUTPUTS/30-node-A-capture.txt | diff -B - 30-node-A-capture.txt


clean::
	@rm -rf 30-ackdao *.o

update:
	cp ../OUTPUTS/29-node-E-out.pcap ../INPUTS/29-dao-nopio.pcap
	cp ../OUTPUTS/31-node-E-out.pcap ../INPUTS/31-dao-extra.pcap
	cp ../OUTPUTS/32-node-E-out.pcap ../INPUTS/32-dao-onlyif.pcap
	cp ../OUTPUTS/30-ackdao-output.raw  30-ackdao-output.txt
	cp ../OUTPUTS/30-node-A-capture.txt 30-node-A-capture.txt

