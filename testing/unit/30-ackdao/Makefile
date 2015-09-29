UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

30-daoack: 30-daoack.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 30-daoack 30-daoack.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 30-daoack
programs::

check::	30-daoack
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13:
	./30-daoack ../INPUTS/29-dao-nopio.pcap 2>&1 | tee ../OUTPUTS/30-daoack-output.raw | diff -B - 30-daoack-output.txt
	@${TCPDUMP} -t -v -n -r ../OUTPUTS/30-node-A-out.pcap | tee ../OUTPUTS/30-node-A-capture.txt | diff -B - 30-node-A-capture.txt


clean::
	@rm -rf 30-daoack *.o

update:
	cp ../OUTPUTS/30-daoack-output.raw  30-daoack-output.txt
	cp ../OUTPUTS/30-node-A-capture.txt 30-node-A-capture.txt

