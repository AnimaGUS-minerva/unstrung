PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

21-nodetests: 21-nodetests.cpp ${INC}/iface.h ${INC}/dag.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o 21-nodetests 21-nodetests.cpp ${LDFLAGS}

check::	21-nodetests
	./21-nodetests

checkprograms:: 21-nodetests
programs::

clean::
	@rm -rf 21-nodetests *.o
