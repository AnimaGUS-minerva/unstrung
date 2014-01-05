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


PANDORA_SRCDIR?=$(shell cd ../..; pwd)
export PANDORA_SRCDIR

include ${PANDORA_SRCDIR}/Makefile.inc

srcdir?=$(shell pwd)

SUBDIRS=01-osw
SUBDIRS+=02-recvdio
SUBDIRS+=03-dioopt
SUBDIRS+=04-dag 05-dag 06-dag
SUBDIRS+=07-dagset
SUBDIRS+=08-event
# needs print_linkinfo
#SUBDIRS+=09-netlink
# fix this, ticket #2939
#SUBDIRS+=10-netlink
SUBDIRS+=12-senddio
SUBDIRS+=13-selfprefix
SUBDIRS+=14-senddao
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
SUBDIRS+=26-senddaoack




