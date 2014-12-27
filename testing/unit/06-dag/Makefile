UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

dag-03: dag-03.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o dag-03 dag-03.cpp ${LDFLAGS}

check::	dag-03
	./dag-03

checkprograms:: dag-03
programs::

clean::
	@rm -rf dag-03 *.o
