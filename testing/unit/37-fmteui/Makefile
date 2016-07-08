UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

fmteui: fmteui.cpp ${INC}/iface.h ${LIBNDMGMT}
	${CXX} ${CXXFLAGS} -o fmteui fmteui.cpp ${LDFLAGS}

check::	fmteui
	./fmteui

checkprograms:: fmteui
programs::

clean::
	@rm -rf fmteui *.o
