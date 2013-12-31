PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

dag-03: dag-03.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o dag-03 dag-03.cpp ${LDFLAGS}

check::	dag-03
	./dag-03

checkprograms:: dag-03
programs::

clean::
	@rm -rf dag-03 *.o
