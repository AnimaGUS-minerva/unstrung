# unstrung master makefile
#
# Copyright (C) 2009 Michael Richardson <mcr@sandelman.ca>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#


UNSTRUNG_SRCDIR?=$(shell cd ../..; pwd)
export UNSTRUNG_SRCDIR

include ${UNSTRUNG_SRCDIR}/Makefile.inc

srcdir?=$(shell pwd)

# make loop add error codes, but doesn't work yet, use checkstop.
#ACCUMULATE_ERRORS=true

SUBDIRS=01-osw
SUBDIRS+=03-dioopt
SUBDIRS+=04-dag 05-dag 06-dag
SUBDIRS+=07-dagset
SUBDIRS+=08-event
# needs print_linkinfo
#SUBDIRS+=09-netlink
# fix this, ticket #2939
#SUBDIRS+=10-netlink
SUBDIRS+=11-senddao
SUBDIRS+=12-senddio
SUBDIRS+=13-selfprefix
SUBDIRS+=14-sunshine
SUBDIRS+=15-builddao
SUBDIRS+=16-vf
SUBDIRS+=17-recvdao
SUBDIRS+=18-checksum
SUBDIRS+=19-pickdag
SUBDIRS+=20-prefix
SUBDIRS+=21-node
SUBDIRS+=22-queues
SUBDIRS+=23-selfdio
SUBDIRS+=24-daoreply
SUBDIRS+=25-childdio
SUBDIRS+=38-builddaoack
SUBDIRS+=26-senddaoack
SUBDIRS+=27-daoreplydao
SUBDIRS+=28-daowrongack
SUBDIRS+=29-daoreply-nopio
SUBDIRS+=30-ackdao
SUBDIRS+=31-daoreply-extraprefix
SUBDIRS+=32-daoreply-onlyif
SUBDIRS+=33-newifdao
SUBDIRS+=34-builddis
SUBDIRS+=35-recvdis
ifeq ($(USE_MBEDTLS),true)
SUBDIRS+=36-peckusage
endif
SUBDIRS+=37-fmteui
SUBDIRS+=38-builddaoack
SUBDIRS+=39-daoJ
SUBDIRS+=40-build-ns
ifeq ($(GRASP_CLIENT),true)
SUBDIRS+=41-acceptns
SUBDIRS+=43-sendgrasp
endif
SUBDIRS+=42-build-na
SUBDIRS+=44-dio-ungrounded
ifeq ($(USE_MBEDTLS),true)
SUBDIRS+=45-extract-eui
endif
SUBDIRS+=46-method-varargs
SUBDIRS+=50-sunshine-root

checkstop::
	@for d in $(SUBDIRS) ; \
	do \
		if $(MAKE) -C $$d UNSTRUNG_SRCDIR=${UNSTRUNG_SRCDIR} check; then true; else echo ${SUBDIR} FAILED; exit 1; fi; \
	done;


