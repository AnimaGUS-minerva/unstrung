UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

24-daoreply: 24-daoreply.cpp ../24-daoreply/dioA.cpp ../24-daoreply/daoackA.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 24-daoreply 24-daoreply.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 24-daoreply
programs::

check::	24-daoreply
	@mkdir -p ../OUTPUTS
	./24-daoreply 2>&1 | tee ../OUTPUTS/24-daoreply-out.raw | diff -B - 24-daoreply-out.txt
	${TCPDUMP} -t -v -n -r ../OUTPUTS/24-node-E-out.pcap | tee ../OUTPUTS/24-node-E-capture.txt | diff -B - 24-node-E-capture.txt

clean::
	@rm -rf 24-daoreply *.o

update:
	cp  ../OUTPUTS/24-daoreply-out.raw   24-daoreply-out.txt
	cp  ../OUTPUTS/24-node-E-capture.txt 24-node-E-capture.txt
