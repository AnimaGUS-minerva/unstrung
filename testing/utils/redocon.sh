#!/bin/sh

if [ -z "${UNSTRUNG_SRCDIR}" ]
then
    if [ -f ../../umlsetup.sh ]; then
        UNSTRUNG_SRCDIR=`cd ../.. && pwd`
    else 
        if [ -f ../../../umlsetup.sh ]; then 
	    UNSTRUNG_SRCDIR=`cd ../../.. && pwd`
        fi
    fi  	
fi

. ../../../umlsetup.sh
TESTINGROOT=${UNSTRUNG_SRCDIR}/testing
UTILS=`cd ${TESTINGROOT}/utils && pwd`

. $UTILS/testing-functions.sh
. testparams.sh

compareoutputs;

