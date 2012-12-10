UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

dioopt-01: dioopt-01.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o dioopt-01 dioopt-01.cpp ${LDFLAGS}

check::	dioopt-01
	./dioopt-01

checkprograms:: dioopt-01
programs::

clean::
	@rm -rf dioopt-01 *.o
