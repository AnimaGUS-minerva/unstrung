UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc

# local install of pcap.h, or make sure you have libpcap-dev installed.
CXXFLAGS+=-I/sandel/include -g
LDFLAGS+=-L${UNSTRUNG_OBJDIR}/lib/libndmgmt -lndmgmt
#LDFLAGS+=/sandel/lib/libpcap.a
LDFLAGS+=/corp/projects/Misc/tcpdump/libpcap/libpcap.a

recv-01: recv-01.cpp
	${CXX} ${CXXFLAGS} -o recv-01 recv-01.cpp ${LDFLAGS}

check:	recv-01
	./recv-01 ../INPUTS/plainnd.pcap | diff -B - recv-01-output.txt
	tcpdump -t -n -r ../OUTPUTS/recv-01-plainnd.pcap -s 0 | diff -B - recv-01-plainnd-out.txt

clean:
	rm -rf recv-01 *.o
