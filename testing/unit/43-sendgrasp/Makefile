UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: 43-sendgrasp

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

RAWPCAP=../OUTPUTS/43-sendgrasp-out.pcap
RAWPCAPOUT=../OUTPUTS/43-sendgrasp-out.txt
RAWGRASPOUT=../OUTPUTS/43-grasp-pretty.txt

43-sendgrasp: 43-sendgrasp.cpp ${LIBS}
	${CXX} ${CXXFLAGS} -DRAWPCAP=\"${RAWPCAP}\" -o 43-sendgrasp 43-sendgrasp.cpp ${LDFLAGS} ${LIBS} ${CBOR_LIB}

checkprograms:: 43-sendgrasp
programs::

check::	43-sendgrasp
	@mkdir -p ../OUTPUTS
	./43-sendgrasp 2>&1 | tee ../OUTPUTS/43-sendgrasp-out.raw | diff -B - 43-sendgrasp-out.txt
	-cbor-pretty ../OUTPUTS/43-6join-grasp.dump | tee ${RAWGRASPOUT} | diff -B - 43-grasp-out.txt

clean::
	@rm -rf 43-sendgrasp *.o

update:
	cp  ../OUTPUTS/43-sendgrasp-out.raw       43-sendgrasp-out.txt
	cp  ${RAWGRASPOUT}                        43-grasp-out.txt
	: cp  ${RAWPCAPOUT}                        43-sendgrasp-capture.txt
