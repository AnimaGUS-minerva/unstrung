# -*- makefile -*-

default:: programs

# stuff.
INC=${UNSTRUNG_SRCDIR}/include
CROSSFLAGS=-m32
CFLAGS+=-I${INC} 
CXXFLAGS+=-I${INC} 
OSDEP=$(shell arch)

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
LIBFAKEIFACE=${UNSTRUNG_OBJDIR}/lib/libfakeiface/libfakeiface.a
LIBPCAP=/usr/local/ia32/lib/libpcap.a
#LIBPCAP=-L/usr/local/ia32/lib -lpcap
# needed by libpcap
EXTRALIBS=-lusb-1.0

NEEDCHECK?=check

clean ${NEEDCHECK} programs checkprograms::
	@for d in $(SUBDIRS) ; \
	do \
		(cd $$d && $(MAKE) srcdir=${srcdir}/$$d/ UNSTRUNG_SRCDIR=${UNSTRUNG_SRCDIR} $@ ) || exit 1; \
	done; 

env:
	@env | sed -e "s/'/'\\\\''/g" -e "s/\([^=]*\)=\(.*\)/\1='\2'/"

