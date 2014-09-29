# -*- makefile -*-

-include ${PANDORA_SRCDIR}/Makefile.local

default:: programs

# stuff.
INC=${PANDORA_SRCDIR}/include
#-I${PANDORA_SRCDIR}/../tcpdump-4.6.0
CROSSFLAGS=-m32
HOSTFLAGS=-m32
CFLAGS+=-I${INC}
CXXFLAGS+=-I${INC}
OSDEP=$(shell uname -s).$(shell arch)

USE_OBJDIR=false
PANDORA_OBJDIR=${PANDORA_SRCDIR}/.
XMLTO=xmlto

MANTREE=${DESTDIR}/usr/share/man
INSTALL=install
INSTMANFLAGS=-D
INSTBINFLAGS=-D

LIBNDMGMT=${PANDORA_OBJDIR}/lib/libndmgmt/libndmgmt.a
LIBOSW=${PANDORA_OBJDIR}/lib/libosw/libosw.a
LIBNETLINK=${PANDORA_OBJDIR}/lib/libnetlink/libnetlinkstrung.a
LIBFAKEIFACE=${PANDORA_OBJDIR}/lib/libfakeiface/libfakeiface.a
#LIBPCAP=/usr/local/ia32/lib/libpcap.a
LIBPCAP?=-L/usr/local/ia32/lib -lpcap
LIBPCAPINC?=-I/sandel/include
TCPDUMP?=tcpdump
# needed by libpcap
EXTRALIBS:=${LIBPCAP} -L/sandel/lib/i386-linux-gnu -lusb-1.0

NETDISSECTLIB?=-L${UNSTRUNG_SRCDIR}/../tcpdump-4.6.0-host -lnetdissect
NETDISSECTH?=-DNETDISSECT -I${UNSTRUNG_SRCDIR}/../tcpdump-4.6.0-host

NEEDCHECK?=check

TAGS clean ${NEEDCHECK} programs checkprograms::
	@err=0; for d in $(SUBDIRS) ; \
	do \
		if $(MAKE) -C $$d PANDORA_SRCDIR=${PANDORA_SRCDIR} $@; then true; else err=$$(($$err + 1)); fi; \
	done; exit $$err

env:
	@env | sed -e "s/'/'\\\\''/g" -e "s/\([^=]*\)=\(.*\)/\1='\2'/"

TAGS::
	@if [ -n "${CXXSRCS}${SRCS}" ]; then etags --append -o ${PANDORA_SRCDIR}/TAGS ${CXXSRCS} ${SRCS}; fi

