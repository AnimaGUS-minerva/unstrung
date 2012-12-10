UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

findeui: findeui.cpp ${LIBNETLINK} ${LIBOSW}
	${CXX} ${CXXFLAGS} -o findeui findeui.cpp ${LDFLAGS}

check::	findeui
	./findeui 2>&1 | diff - marajade.output.txt

checkprograms:: findeui
programs::

clean::
	@rm -rf findeui *.o
