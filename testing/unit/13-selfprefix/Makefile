UNSTRUNG_SRCDIR?=$(shell cd ../../..; pwd)

include ${UNSTRUNG_SRCDIR}/Makefile.inc
include ${srcdir}../Makefile.testing

selfprefix-13: selfprefix-13.cpp ${INC}/iface.h ${LIBS}
	${CXX} ${CXXFLAGS} -o selfprefix-13 selfprefix-13.cpp ${LDFLAGS}

checkprograms:: selfprefix-13
programs::

check::	selfprefix-13
	@mkdir -p ../OUTPUTS
	@echo SELFPREFIX13:
	./selfprefix-13 2>&1 | tee ../OUTPUTS/13-selfprefix-output.raw | diff -B - selfprefix-13-output.txt

clean::
	@rm -rf selfprefix-13 *.o

update::
	cp ../OUTPUTS/13-selfprefix-output.raw selfprefix-13-output.txt
