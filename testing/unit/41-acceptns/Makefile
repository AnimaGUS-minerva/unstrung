UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: 41-acceptns

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

RAWPCAP=../OUTPUTS/41-acceptns-out.pcap
RAWPCAPOUT=../OUTPUTS/41-acceptns-out.txt

41-acceptns: 41-acceptns.cpp ${LIBS}
	${CXX} ${CXXFLAGS} -DRAWPCAP=\"${RAWPCAP}\" -o 41-acceptns 41-acceptns.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 41-acceptns
programs::

check::	41-acceptns
	@mkdir -p ../OUTPUTS
	./41-acceptns ../INPUTS/nodeM-ns-tooshort.pcap 2>&1 | tee ../OUTPUTS/41-acceptns-tooshort.raw | diff -B - 41-acceptns-tooshort.txt
	./41-acceptns 2>&1 | tee ../OUTPUTS/41-acceptns-out.raw | diff -B - 41-acceptns-out.txt
	${TCPDUMP} -t -v -v -n -r ${RAWPCAP} | tee ${RAWPCAPOUT} | diff -B - 41-acceptns-capture.txt

clean::
	@rm -rf 41-acceptns *.o

update:
	cp  ../OUTPUTS/41-acceptns-tooshort.raw  41-acceptns-tooshort.txt
	cp  ${RAWPCAPOUT}                        41-acceptns-capture.txt
	cp  ../OUTPUTS/41-acceptns-out.raw       41-acceptns-out.txt
	cp  ${RAWPCAPOUT}                        41-acceptns-capture.txt
