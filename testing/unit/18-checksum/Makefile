PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

18-checksum: 18-checksum.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 18-checksum 18-checksum.cpp ${LDFLAGS}

checkprograms:: 18-checksum
programs::

check::	18-checksum
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13:
	./18-checksum
	@echo SUCCEED

clean::
	@rm -rf 18-checksum *.o
