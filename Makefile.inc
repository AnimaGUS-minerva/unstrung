# -*- makefile -*-

-include ${UNSTRUNG_SRCDIR}/Makefile.local

default:: programs

# stuff to set... use -m32 by default because of UML testing, should remove this to elsewhere.
INC=${UNSTRUNG_SRCDIR}/include
#-I${UNSTRUNG_SRCDIR}/../tcpdump-4.6.0
#CROSSFLAGS?=-m32
HOSTFLAGS=-m32
CFLAGS+=-I${INC}
CXXFLAGS+=-I${INC}
OSDEP=$(shell uname -s).$(shell arch)

USE_OBJDIR?=false
XMLTO=xmlto

MANTREE=${DESTDIR}/usr/share/man
INSTALL=install
INSTMANFLAGS=-D
INSTBINFLAGS=-D

ifneq ($(EMBEDDED),)
CFLAGS+=-DEMBEDDED
endif

ifeq ($(USE_OBJDIR),true)
ARCH?=$(shell arch)
OBJDIR?=OBJ.${ARCH}
OBJDIRTOP?=$(shell mkdir -p ${OBJDIR}; cd ${OBJDIR} && pwd)
UNSTRUNG_OBJDIR?=${OBJDIRTOP}
export OBJDIRTOP
else
UNSTRUNG_OBJDIR?=.
endif

LIBNDMGMT=${UNSTRUNG_OBJDIR}/lib/libndmgmt/libndmgmt.a
LIBOSW=${UNSTRUNG_OBJDIR}/lib/libosw/libosw.a
LIBNETLINK=${UNSTRUNG_OBJDIR}/lib/libnetlink/libnetlinkstrung.a
LIBFAKEIFACE=${UNSTRUNG_OBJDIR}/lib/libfakeiface/libfakeiface.a
#LIBPCAP=/usr/local/ia32/lib/libpcap.a
LIBPCAP?=-L/usr/local/ia32/lib -lpcap
LIBPCAPINC?=-I/sandel/include
TCPDUMP?=tcpdump

ifeq ($(EMBEDDED),)
# needed by fake interface library
EXTRALIBS:=${LIBPCAP} -L/sandel/lib/i386-linux-gnu -lusb-1.0
endif

NETDISSECTLIB?=-L${UNSTRUNG_SRCDIR}/../tcpdump-4.6.0-host -lnetdissect
NETDISSECTH?=-DNETDISSECT -I${UNSTRUNG_SRCDIR}/../tcpdump-4.6.0-host

NEEDCHECK?=check

.PHONY: TAGS clean ${NEEDCHECK} programs checkprograms install

ifeq (${MAKEFILETOP},)
TAGS clean ${NEEDCHECK} programs checkprograms install::
	@err=0; for d in $(SUBDIRS) ; \
	do \
		if $(MAKE) -C $$d UNSTRUNG_SRCDIR=${UNSTRUNG_SRCDIR} $@; then true; else err=$$(($$err + 1)); fi; \
	done; exit $$err
endif

env:
	@env | sed -e "s/'/'\\\\''/g" -e "s/\([^=]*\)=\(.*\)/\1='\2'/"

TAGS::
	@if [ -n "${CXXSRCS}${SRCS}" ]; then etags --append -o ${UNSTRUNG_SRCDIR}/TAGS ${CXXSRCS} ${SRCS}; fi

