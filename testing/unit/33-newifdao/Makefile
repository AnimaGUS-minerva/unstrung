UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

33-newifdao: 33-newifdao.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 33-newifdao 33-newifdao.cpp ${LDFLAGS} ${LIBS}

checkprograms:: 33-newifdao
programs::

check::	33-newifdao
	@mkdir -p ../OUTPUTS
	./33-newifdao 2>&1 | tee ../OUTPUTS/33-newifdao-out.raw | diff -B - 33-newifdao-out.txt

clean::
	@rm -rf 33-newifdao *.o

update:
	cp  ../OUTPUTS/33-newifdao-out.raw  33-newifdao-out.txt

