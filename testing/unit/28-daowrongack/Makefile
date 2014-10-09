PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

28-daowrongack: 28-daowrongack.cpp ../24-daoreply/dioA.cpp ../24-daoreply/daoackA.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 28-daowrongack 28-daowrongack.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 28-daowrongack
programs::

check::	28-daowrongack
	@mkdir -p ../OUTPUTS
	./28-daowrongack 2>&1 | tee ../OUTPUTS/28-daowrongack-out.raw | diff -B - 28-daowrongack-out.txt
	${TCPDUMP} -t -v -n -r ../OUTPUTS/24-node-E-out.pcap | tee ../OUTPUTS/28-node-E-capture.txt | diff -B - 28-node-E-capture.txt

clean::
	@rm -rf 28-daowrongack *.o

update:
	cp  ../OUTPUTS/28-daowrongack-out.raw   28-daowrongack-out.txt
	cp  ../OUTPUTS/28-node-E-capture.txt    28-node-E-capture.txt
