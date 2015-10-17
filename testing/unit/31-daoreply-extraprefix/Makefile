UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

31-daoreply-extra: 31-daoreply-extra.cpp ../24-daoreply/dioA.cpp ../24-daoreply/daoackA.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 31-daoreply-extra 31-daoreply-extra.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 31-daoreply-extra
programs::

check::	31-daoreply-extra
	@mkdir -p ../OUTPUTS
	./31-daoreply-extra 2>&1 | tee ../OUTPUTS/31-daoreply-extra-out.raw | diff -B - 31-daoreply-extra-out.txt
	${TCPDUMP} -t -v -v -n -r ../OUTPUTS/31-node-E-out.pcap | tee ../OUTPUTS/31-node-E-capture.txt | diff -B - 31-node-E-capture.txt

clean::
	@rm -rf 31-daoreply-extra *.o

update:
	cp  ../OUTPUTS/31-daoreply-extra-out.raw   31-daoreply-extra-out.txt
	cp  ../OUTPUTS/31-node-E-capture.txt       31-node-E-capture.txt
