UNSTRUNG_SRCDIR?=$(shell cd ../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc

# local install of pcap.h, or make sure you have libpcap-dev installed.
CFLAGS+=-I/sandel/include -g

sendra: sendra.c 
	${CC} ${CFLAGS} -o sendra sendra.c ${LDFLAGS}

clean:
	rm -rf sendra *.o
