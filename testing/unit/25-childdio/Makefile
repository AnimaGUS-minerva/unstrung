UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

25-childdio: 25-childdio.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o 25-childdio 25-childdio.cpp ${LDFLAGS}

checkprograms:: 25-childdio
programs::

check::	25-childdio
	@mkdir -p ../OUTPUTS
	@echo 25-CHILDDIO:
	./25-childdio 2>&1 | tee ../OUTPUTS/25-childdio-output.raw | diff -B - 25-childdio-output.txt

clean::
	@rm -rf 25-childdio *.o

update::
	cp ../OUTPUTS/25-childdio-output.raw 25-childdio-output.txt
