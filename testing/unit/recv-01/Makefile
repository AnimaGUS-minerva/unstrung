UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
INC=${UNSTRUNG_SRCDIR}/include

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LIBNDMGMT=${UNSTRUNG_OBJDIR}/lib/libndmgmt/libndmgmt.a
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt
#LDFLAGS+=/sandel/lib/libpcap.a
LDFLAGS+=/corp/projects/Misc/tcpdump/libpcap/libpcap.a

recv-01: recv-01.cpp ${INC}/iface.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o recv-01 recv-01.cpp ${LDFLAGS}

check:	recv-01
	@echo TEST01: PLAINND
	./recv-01 ../INPUTS/plainnd.pcap | tee ../OUTPUTS/recv-01-output.raw | diff -B - recv-01-output.txt
	tcpdump -t -n -r ../OUTPUTS/recv-01-plainnd.pcap -s 0 | diff -B - recv-01-plainnd-out.txt
	@echo TEST02: SENDDIO CREATED
	./recv-01 ../INPUTS/senddio.pcap | tee ../OUTPUTS/recv-02-output.raw | diff -B - recv-02-output.txt
	tcpdump -t -n -r ../OUTPUTS/recv-01-senddio.pcap -s 0 | diff -B - recv-01-senddio-out.txt

clean:
	rm -rf recv-01 *.o
