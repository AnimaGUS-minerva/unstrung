UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

all: check

OSWDIR=${UNSTRUNG_SRCDIR}/lib/libosw

addrtot-01: ${OSWDIR}/addrtot.c
	${CC} -DADDRTOT_MAIN ${CFLAGS} -o addrtot-01 ${OSWDIR}/addrtot.c ${LDFLAGS}

ttoaddr-01: ${OSWDIR}/ttoaddr.c 
	${CC} -DTTOADDR_MAIN ${CFLAGS} -o ttoaddr-01 ${OSWDIR}/ttoaddr.c ${LDFLAGS}

ttosubnet-01: ${OSWDIR}/ttosubnet.c
	${CC} -DTTOSUBNET_MAIN ${CFLAGS} -o ttosubnet-01 ${OSWDIR}/ttosubnet.c ${LDFLAGS}

check::	addrtot-01 ttoaddr-01 ttosubnet-01
	./addrtot-01 -r
	./ttoaddr-01 -r
	./ttosubnet-01 -r

checkprograms:: 
programs::

clean::
	@rm -rf addrtot-01 ttoaddr-01 ttosubnet-01 *.o
