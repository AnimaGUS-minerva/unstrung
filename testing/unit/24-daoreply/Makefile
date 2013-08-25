UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

24-daoreply: 24-daoreply.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 24-daoreply 24-daoreply.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 24-daoreply
programs::

check::	24-daoreply
	@mkdir -p ../OUTPUTS
	./24-daoreply 2>&1 | tee ../OUTPUTS/24-daoreply-out.raw | diff -B - 24-daoreply-out.txt

clean::
	@rm -rf 24-daoreply *.o
