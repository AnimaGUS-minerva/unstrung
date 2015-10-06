UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

32-daoreply-onlyif: 32-daoreply-onlyif.cpp ../24-daoreply/dioA.cpp ../24-daoreply/daoackA.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 32-daoreply-onlyif 32-daoreply-onlyif.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 32-daoreply-onlyif
programs::

check::	32-daoreply-onlyif
	@mkdir -p ../OUTPUTS
	./32-daoreply-onlyif 2>&1 | tee ../OUTPUTS/32-daoreply-onlyif-out.raw | diff -B - 32-daoreply-onlyif-out.txt
	${TCPDUMP} -t -v -v -n -r ../OUTPUTS/32-node-E-out.pcap | tee ../OUTPUTS/32-node-E-capture.txt | diff -B - 32-node-E-capture.txt

clean::
	@rm -rf 32-daoreply-onlyif *.o

update:
	cp  ../OUTPUTS/32-daoreply-onlyif-out.raw  32-daoreply-onlyif-out.txt
	cp  ../OUTPUTS/32-node-E-capture.txt       32-node-E-capture.txt

