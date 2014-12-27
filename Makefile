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


UNSTRUNG_SRCDIR?=$(shell pwd)
export UNSTRUNG_SRCDIR

TERMCAP=
export TERMCAP

default:: programs

srcdir?=$(shell pwd)

-include ${UNSTRUNG_SRCDIR}/Makefile.vendor

include ${UNSTRUNG_SRCDIR}/Makefile.ver
include ${UNSTRUNG_SRCDIR}/Makefile.top
include ${UNSTRUNG_SRCDIR}/Makefile.inc

# used by OpenWRT build process.
ifneq ($(USE_OBJDIR),true)
TAGS clean ${NEEDCHECK} programs checkprograms install::
	@err=0; for d in $(SUBDIRS) ; \
	do \
		if $(MAKE) -C $$d UNSTRUNG_SRCDIR=${UNSTRUNG_SRCDIR} $@; then true; else err=$$(($$err + 1)); fi; \
	done; exit $$err
else

ABSOBJDIR?=$(shell mkdir -p ${OBJDIR} && cd ${OBJDIR} && pwd)
TAGS clean ${NEEDCHECK} programs checkprograms install:: ${OBJDIRTOP}/Makefile
	(cd ${ABSOBJDIR} && OBJDIRTOP=${ABSOBJDIR} OBJDIR=${ABSOBJDIR} ${MAKE} $@ )

${ABSOBJDIR}/Makefile: ${UNSTRUNG_SRCDIR}/packaging/utils/makeshadowdir
	@echo Setting up for OBJDIR=${OBJDIR}
	@${UNSTRUNG_SRCDIR}/packaging/utils/makeshadowdir `(cd ${srcdir}; echo $$PWD)` ${OBJDIR} "${SUBDIRS}"

endif



