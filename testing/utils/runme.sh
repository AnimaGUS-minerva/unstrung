#!/bin/sh

#
# $Id: runme.sh,v 1.3 2004/05/28 02:07:02 mcr Exp $
#
# use this script to run a single test from within that test directory.
#

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

if [ -z "${TEST_TYPE}" ]
then
    echo runme.sh now requires that testparams.sh defines TEST_TYPE=
fi

( cd .. && $TEST_TYPE $TESTNAME good )

summarize_results


