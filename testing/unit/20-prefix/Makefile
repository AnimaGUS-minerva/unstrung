UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

20-prefixtests: 20-prefixtests.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o 20-prefixtests 20-prefixtests.cpp ${LDFLAGS}

check::	20-prefixtests
	./20-prefixtests

checkprograms:: 20-prefixtests
programs::

clean::
	@rm -rf 20-prefixtests *.o
