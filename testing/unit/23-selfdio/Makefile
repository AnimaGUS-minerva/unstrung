PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

23-selfdio: 23-selfdio.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 23-selfdio 23-selfdio.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 23-selfdio
programs::

check::	23-selfdio
	@mkdir -p ../OUTPUTS
	./23-selfdio 2>&1 | tee ../OUTPUTS/23-selfdio-out.raw | diff -B - 23-selfdio-out.txt

clean::
	@rm -rf 23-selfdio *.o
