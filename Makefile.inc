# -*- makefile -*-

-include ${UNSTRUNG_SRCDIR}/Makefile.local

default:: programs

ARCH=$(shell uname -m)
GRASP_CLIENT?=false
USE_MBEDTLS?=false

# stuff to set... use -m32 by default because of UML testing, should remove this to elsewhere.
INC=${UNSTRUNG_SRCDIR}/include
#-I${UNSTRUNG_SRCDIR}/../tcpdump-4.6.0
CFLAGS+=-I${INC} -fno-show-column
CXXFLAGS+=-I${INC}  -fno-show-column
USERLINK?=-g
USERCPPCOMPILE?=-g -O0

#ifeq ($(ARCH),x86_64)
#HOSTFLAGS=-m32
#endif

USE_OBJDIR?=false
XMLTO=xmlto
RONN=ronn
MANTREE=/usr/share/man
INSTALL=install
INSTMANFLAGS=-D
INSTBINFLAGS=-D

ifneq ($(EMBEDDED),)
CFLAGS+=-DEMBEDDED
CXXFLAGS+=-DEMBEDDED
endif

# no grasp for now.
ifeq ($(GRASP_CLIENT),true)
CFLAGS+= -DGRASP_CLIENT
CXXFLAGS+= -DGRASP_CLIENT
LIBGRASP=${UNSTRUNG_OBJDIR}/lib/libgrasp/libgrasp.a
endif

ifeq ($(USE_MBEDTLS),true)
CFLAGS+= ${MBEDTLSH} -DHAVE_MBEDTLS
CXXFLAGS+= ${MBEDTLSH} -DHAVE_MBEDTLS
LIBCRYPT=${MBEDTLSLIB}/libmbedtls.a
LIBCRYPT+=${MBEDTLSLIB}/libmbedcrypto.a
LIBCRYPT+=${MBEDTLSLIB}/libmbedx509.a
else
# no mbedtls
LIBCRYPT=
endif

CFLAGS+= ${CBOR_INCLUDE}
CXXFLAGS+= ${CBOR_INCLUDE}

ifeq ($(USE_OBJDIR),true)
OBJDIR?=OBJ.${ARCH}
else
OBJDIR=.
endif
OBJDIRTOP?=$(shell cd ${UNSTRUNG_SRCDIR} && mkdir -p ${OBJDIR}; cd ${OBJDIR} && pwd)
UNSTRUNG_OBJDIR?=${OBJDIRTOP}
export OBJDIRTOP
OSDEP=$(shell uname -s).${ARCH}

ifeq ($(ARCH),i386)
CROSSFLAGS?=-m32
endif

LIBNDMGMT=${UNSTRUNG_OBJDIR}/lib/libndmgmt/libndmgmt.a
LIBOSW=${UNSTRUNG_OBJDIR}/lib/libosw/libosw.a
LIBNETLINK=${UNSTRUNG_OBJDIR}/lib/libnetlink/libnetlinkstrung.a
LIBIDEVID=${UNSTRUNG_OBJDIR}/lib/libidevid/libidevid.a
LIBFAKEIFACE=${UNSTRUNG_OBJDIR}/lib/libfakeiface/libfakeiface.a
LIBPCAP?=$(shell pcap-config --libs --static)
LIBPCAP_HOST?=-lpcap
LIBCBOR=${CBOR_LIB}
TCPDUMP?=tcpdump

ifeq ($(EMBEDDED),)
# needed by fake interface library
EXTRALIBS:=${LIBPCAP} ${EXTRALIBPATHS} -lusb -lpthread
endif

NETDISSECTLIB?=-L${UNSTRUNG_SRCDIR}/../amd64/tcpdump -lnetdissect
NETDISSECTH?=-DNETDISSECT -I${UNSTRUNG_SRCDIR}/../tcpdump

NEEDCHECK?=check

.PHONY: TAGS clean ${NEEDCHECK} programs checkprograms install

ifeq (${MAKEFILETOP},)
TAGS clean ${NEEDCHECK} programs checkprograms install::
	@err=0; for d in $(SUBDIRS) ; \
	do \
		if $(MAKE) -C $$d UNSTRUNG_SRCDIR=${UNSTRUNG_SRCDIR} $@; then true; else err=$$(($$err + 1)); fi; \
	done; exit $$err
endif

env::
	@env | sed -e "s/'/'\\\\''/g" -e "s/\([^=]*\)=\(.*\)/\1='\2'/"

TAGS::
	@if [ -n "${CXXSRCS}${SRCS}" ]; then etags --append -o ${UNSTRUNG_SRCDIR}/TAGS ${CXXSRCS} ${SRCS}; fi

