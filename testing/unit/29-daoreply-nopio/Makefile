UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

29-daoreply-nopio: 29-daoreply-nopio.cpp ../24-daoreply/dioA.cpp ../24-daoreply/daoackA.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 29-daoreply-nopio 29-daoreply-nopio.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 29-daoreply-nopio
programs::

check::	29-daoreply-nopio
	@mkdir -p ../OUTPUTS
	./29-daoreply-nopio 2>&1 | tee ../OUTPUTS/29-daoreply-nopio-out.raw | diff -B - 29-daoreply-nopio-out.txt
	${TCPDUMP} -t -v -n -r ../OUTPUTS/24-node-E-out.pcap | tee ../OUTPUTS/29-node-E-capture.txt | diff -B - 29-node-E-capture.txt

clean::
	@rm -rf 29-daoreply-nopio *.o

update:
	cp  ../OUTPUTS/29-daoreply-nopio-out.raw   29-daoreply-nopio-out.txt
	cp  ../OUTPUTS/29-node-E-capture.txt       29-node-E-capture.txt
