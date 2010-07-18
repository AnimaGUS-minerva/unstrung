# -*- makefile -*-

default:: programs

# stuff.
INC=${UNSTRUNG_SRCDIR}/include
CFLAGS+=-I${INC}
CXXFLAGS+=-I${INC}

USE_OBJDIR=false
UNSTRUNG_OBJDIR=${UNSTRUNG_SRCDIR}/.
XMLTO=xmlto

MANTREE=${DESTDIR}/usr/share/man
INSTALL=install
INSTMANFLAGS=-D
INSTBINFLAGS=-D

LIBNDMGMT=${UNSTRUNG_OBJDIR}/lib/libndmgmt/libndmgmt.a
LIBOSW=${UNSTRUNG_OBJDIR}/lib/libosw/libosw.a
LIBNETLINK=${UNSTRUNG_OBJDIR}/lib/libnetlink/libnetlinkstrung.a

NEEDCHECK?=check

clean ${NEEDCHECK} programs checkprograms::
	@for d in $(SUBDIRS) ; \
	do \
		(cd $$d && $(MAKE) srcdir=${srcdir}/$$d/ UNSTRUNG_SRCDIR=${UNSTRUNG_SRCDIR} $@ ) || exit 1; \
	done; 

env:
	@env | sed -e "s/'/'\\\\''/g" -e "s/\([^=]*\)=\(.*\)/\1='\2'/"

