UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

18-checksum: 18-checksum.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 18-checksum 18-checksum.cpp ${LDFLAGS}

checkprograms:: 18-checksum
programs::

check::	18-checksum
	@mkdir -p ../OUTPUTS
	@echo 18CHECKSUM:
	./18-checksum
	@echo SUCCEED

clean::
	@rm -rf 18-checksum *.o
