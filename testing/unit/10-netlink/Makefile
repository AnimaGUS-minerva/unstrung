PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

findeui: findeui.cpp ${LIBNETLINK} ${LIBOSW}
	${CXX} ${CXXFLAGS} -o findeui findeui.cpp ${LDFLAGS}

check::	findeui
	./findeui >/dev/null

checkprograms:: findeui
programs::

clean::
	@rm -rf findeui *.o
