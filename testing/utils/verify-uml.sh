#!/bin/sh

# show me
#set -x

# fail if any command fails
set -e

case $# in
    1) PANDORA_SRCDIR=$1; shift;;
esac

if [ `id -u` = 0 ]
then
    echo Do not run this as root.
    exit
fi

#
# configuration for this file has moved to $PANDORA_SRCDIR/umlsetup.sh
# By default, that file does not exist. A sample is at umlsetup-sample.sh
# in this directory. Copy it to $PANDORA_SRCDIR and edit it.
#
if [ -z "${PANDORA_SRCDIR}" ] && [ -f umlsetup.sh ]
then
    PANDORA_SRCDIR=`pwd`
fi

PANDORA_SRCDIR=${PANDORA_SRCDIR-../..}
if [ ! -f ${PANDORA_SRCDIR}/umlsetup.sh ]
then
    echo No umlsetup.sh. Please read instructions in doc/umltesting.html and testing/utils/umlsetup-sample.sh.
    exit 1
fi

. ${PANDORA_SRCDIR}/umlsetup.sh

if [ ! -d ${KERNPOOL}/. ]; then echo Your KERNPOOL= is not properly set; exit 1; fi	

if [ "${UMLPATCH}" != "none" ] && [ ! -r ${UMLPATCH} ]; then echo Your UMLPATCH= is not properly set; exit 1; fi
if [ -z "${PANDORA_HOSTS}" ]; then echo Your PANDORA_HOSTS= is not properly set; exit 1; fi
if [ ! -d ${BASICROOT}/. ]; then echo Your BASICROOT=${BASICROOT} is not properly set; exit 1; fi
    
