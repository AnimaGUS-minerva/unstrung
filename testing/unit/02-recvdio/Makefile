PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

recv-01: recv-01.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o recv-01 recv-01.cpp ${LDFLAGS}

checkprograms:: recv-01
programs::

check::	part01 part02 part03 part04

part01: recv-01
	@mkdir -p ../OUTPUTS
	@echo TEST01: PLAINND
	./recv-01 ../INPUTS/plainnd.pcap | tee ../OUTPUTS/recv-01-output.raw | diff -B - recv-01-output.txt
	tcpdump -t -n -r ../OUTPUTS/recv-01-plainnd.pcap -s 0 | diff -B - recv-01-plainnd-out.txt

part02: recv-01
	@mkdir -p ../OUTPUTS
	@echo TEST02: SENDDIO CREATED
	./recv-01 ../INPUTS/senddio.pcap 2>&1 | tee ../OUTPUTS/recv-02-output.raw | diff -B - recv-02-output.txt

part03: recv-01
	@echo TEST03: SENDDIO-2 CREATED
	./recv-01 ../INPUTS/senddio-2.pcap 2>&1 | tee ../OUTPUTS/recv-03-output.raw | diff -B - recv-03-output.txt

part04: recv-01
	@echo TEST04: SENDDIO-example CREATED
	./recv-01 ../INPUTS/senddio-example.pcap 2>&1 | tee ../OUTPUTS/recv-04-output.raw | diff -B - recv-04-output.txt

clean::
	@rm -rf recv-01 *.o

update::
	cp ../OUTPUTS/recv-01-output.raw recv-01-output.txt
	cp ../OUTPUTS/recv-02-output.raw recv-02-output.txt
	cp ../OUTPUTS/recv-03-output.raw recv-03-output.txt
	cp ../OUTPUTS/recv-04-output.raw recv-04-output.txt


