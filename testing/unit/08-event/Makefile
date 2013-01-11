UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

event-01: event-01.cpp ${INC}/event.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o event-01 event-01.cpp ${LDFLAGS}

check::	event-01
	./event-01

checkprograms:: event-01
programs::

clean::
	@rm -rf event-01 *.o
