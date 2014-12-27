UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

all: check

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

findeui: findeui.cpp ${LIBNETLINK} ${LIBOSW}
	${CXX} ${CXXFLAGS} -o findeui findeui.cpp ${LDFLAGS}

check::	findeui
	./findeui >/dev/null

checkprograms:: findeui
programs::

clean::
	@rm -rf findeui *.o
