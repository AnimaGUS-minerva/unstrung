PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

dag-05: dag-05.cpp ${INC}/iface.h ${INC}/dag.h ${LIBS}
	${CXX} ${CXXFLAGS} -o dag-05 dag-05.cpp ${LDFLAGS}

check::	dag-05
	./dag-05

unitcheck checkprograms:: dag-02
programs::

clean::
	@rm -rf dag-05 *.o
