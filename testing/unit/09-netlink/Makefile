UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

dumpif: dumpif.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o dumpif dumpif.cpp ${LDFLAGS}

check::	dumpif
	./dumpif | grep "lo:"

checkprograms:: dumpif
programs::

clean::
	@rm -rf dumpif *.o
