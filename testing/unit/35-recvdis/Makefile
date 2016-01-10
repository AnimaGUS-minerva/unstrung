UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

35-recvdis: 35-recvdis.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 35-recvdis 35-recvdis.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 35-recvdis
programs::

check::	35-recvdis
	@mkdir -p ../OUTPUTS
	./35-recvdis 2>&1 | tee ../OUTPUTS/35-recvdis-output.raw | diff -B - 35-recvdis-output.txt
	@${TCPDUMP} -t -v -n -r ../OUTPUTS/35-node-A-out.pcap | tee ../OUTPUTS/35-node-A-capture.txt | diff -B - 35-node-A-capture.txt


clean::
	@rm -rf 35-recvdis *.o

update:
	cp ../OUTPUTS/35-recvdis-output.raw  35-recvdis-output.txt
	cp ../OUTPUTS/35-node-A-capture.txt  35-node-A-capture.txt

inputs:
	cp ../OUTPUTS/34-node-E-dis.pcap ../INPUTS/35-nodeE-dis.pcap

# Local Variables:
# compile-command: "make check"
# End:
