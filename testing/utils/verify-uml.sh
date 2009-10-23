#!/bin/sh

# show me
#set -x

# fail if any command fails
set -e

case $# in
    1) UNSTRUNG_SRCDIR=$1; shift;;
esac

if [ `id -u` = 0 ]
then
    echo Do not run this as root.
    exit
fi

#
# configuration for this file has moved to $UNSTRUNG_SRCDIR/umlsetup.sh
# By default, that file does not exist. A sample is at umlsetup-sample.sh
# in this directory. Copy it to $UNSTRUNG_SRCDIR and edit it.
#
if [ -z "${UNSTRUNG_SRCDIR}" ] && [ -f umlsetup.sh ]
then
    UNSTRUNG_SRCDIR=`pwd`
fi

UNSTRUNG_SRCDIR=${UNSTRUNG_SRCDIR-../..}
if [ ! -f ${UNSTRUNG_SRCDIR}/umlsetup.sh ]
then
    echo No umlsetup.sh. Please read instructions in doc/umltesting.html and testing/utils/umlsetup-sample.sh.
    exit 1
fi

. ${UNSTRUNG_SRCDIR}/umlsetup.sh

if [ ! -d ${KERNPOOL}/. ]; then echo Your KERNPOOL= is not properly set; exit 1; fi	

if [ "${UMLPATCH}" != "none" ] && [ ! -r ${UMLPATCH} ]; then echo Your UMLPATCH= is not properly set; exit 1; fi
if [ -z "${UNSTRUNG_HOSTS}" ]; then echo Your UNSTRUNG_HOSTS= is not properly set; exit 1; fi
if [ ! -d ${BASICROOT}/. ]; then echo Your BASICROOT=${BASICROOT} is not properly set; exit 1; fi
    
