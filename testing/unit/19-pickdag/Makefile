UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

19-pickdag: 19-pickdag.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 19-pickdag 19-pickdag.cpp ${LDFLAGS}

checkprograms:: 19-pickdag
programs::

check::	19-pickdag
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13: 
	./19-pickdag 2>&1 | tee ../OUTPUTS/19-pickdag-output.raw | diff -B - 19-pickdag-output.txt

clean::
	@rm -rf 19-pickdag *.o
