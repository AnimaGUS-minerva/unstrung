UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ../Makefile.testing

17-recvdao: 17-recvdao.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 17-recvdao 17-recvdao.cpp ${LDFLAGS}

checkprograms:: 17-recvdao
programs::

check::	17-recvdao
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13: 
	./17-recvdao 2>&1 | tee ../OUTPUTS/17-recvdao-output.raw | diff -B - 17-recvdao-output.txt

clean::
	@rm -rf 17-recvdao *.o
