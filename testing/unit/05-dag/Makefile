UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

dag-02: dag-02.cpp ${INC}/iface.h ${INC}/dag.h ${LIBS}
	${CXX} ${CXXFLAGS} -o dag-02 dag-02.cpp ${LDFLAGS}

check::	dag-02
	./dag-02

checkprograms:: dag-02
programs::

clean::
	@rm -rf dag-02 *.o
