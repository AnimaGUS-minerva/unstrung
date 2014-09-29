PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

17-recvdao: 17-recvdao.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 17-recvdao 17-recvdao.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 17-recvdao
programs::

check::	17-recvdao
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13:
	./17-recvdao 2>&1 | tee ../OUTPUTS/17-recvdao-output.raw | diff -B - 17-recvdao-output.txt
	@${TCPDUMP} -t -v -n -r ../OUTPUTS/17-node-A-out.pcap | tee ../OUTPUTS/17-node-A-capture.txt | diff -B - 17-node-A-capture.txt


clean::
	@rm -rf 17-recvdao *.o

update:
	cp ../OUTPUTS/17-recvdao-output.raw  17-recvdao-output.txt
	cp ../OUTPUTS/17-node-A-capture.txt  17-node-A-capture.txt

