#!/bin/sh

#
# $Id: functions.sh,v 1.131 2005/11/16 21:31:50 mcr Exp $
#

#for debugging set these
#NETJIGVERBOSE=true 
#NETJIGDEBUG=true

SUMMARIZE_RESULTS=${SUMMARIZE_RESULTS-false}
TESTHOST=${TESTHOST-}
NETJIGVERBOSE=${NETJIGVERBOSE-}
THREEEIGHT=${THREEEIGHT-}
TCPDUMP=${TCPDUMP-tcpdump}
TCPDUMPFLAGS=${TCPDUMPFLAGS-}
TEST_GOAL_ITEM=${TEST_GOAL_ITEM-0}
TEST_PROB_REPORT=${TEST_PROB_REPORT-0}
TEST_EXPLOIT_URL=${TEST_EXPLOIT_URL-http://www.openswan.org/vuln/}
TESTUTILS=${UNTI_SRCDIR}/testing/utils
FIXUPDIR=${UNTI_SRCDIR}/testing/utils/fixups
NJ=${TESTUTILS}/uml_netjig/uml_netjig
MAKE=${MAKE-make}

summarize_results() {
    if $SUMMARIZE_RESULTS; then
        perl ${UNTI_SRCDIR}/testing/utils/regress-summarize-results.pl ${REGRESSRESULTS} ${TESTNAME}${KLIPS_MODULE}
    fi
}

preptest() {
    local testdir="$1"
    local testtype="$2"
    local createobjdir="$3"

    if [ ! -r "$testdir/testparams.sh" ]
    then
	echo '      ' "Missing configuration file: $testdir/testparams.sh"
	exit 1
    fi

    createobjdir=${createobjdir-false}

    # make sure no results survive from a past run
    if [ ! -z "$testdir" ] ; then
        if $createobjdir; then
	    rm -rf "$testdir/OUTPUT"${KLIPS_MODULE}
	    mkdir -p "$testdir/OUTPUT"${KLIPS_MODULE}
	fi
    fi

    cd $testdir

    source ./testparams.sh

    if [ "X$TEST_TYPE" != "X$testtype" ]
    then
        echo "Error: TEST_TYPE differs.  Check agreement of TESTLIST and testparams.sh"
	exit 1
    fi

    # Xhost script takes things from the environment.
    for host in $XHOST_LIST
    do
	ROOT=$POOLSPACE/$host/root
	rm -f $ROOT/var/tmp/core
    done
}

lookforcore() {
    local testdir="$1"

    if [ -d "$testdir" ]
    then
	cd $testdir

	if [ -f ./testparams.sh ]
	then
	    source ./testparams.sh
	fi

	# Xhost script takes things from the environment.
	for host in $XHOST_LIST
	do
	    ROOT=$POOLSPACE/$host/root
	    if [ -f $ROOT/var/tmp/core ]
	    then
		mv $ROOT/var/tmp/core OUTPUT${KLIPS_MODULE}/pluto.$host.core
		echo "pluto.$host.core "
	    fi
	done
    fi
}


verboseecho() {
    if [ -n "${NETJIGVERBOSE-}" ]
    then
	echo $@
    fi
}

# ??? NOTE:
# This seems to only sometimes set $success.
# Whatever interesting settings are made seem to be lost by the caller :-(
consolediff() {
    prefix=$1
    output=$2
    ref=$3

    cleanups="cat $output "
    success=${success-true}

    for fixup in `echo $REF_CONSOLE_FIXUPS`
    do
	if [ -f $FIXUPDIR/$fixup ]
	then
	    case $fixup in
		*.sed) cleanups="$cleanups | sed -f $FIXUPDIR/$fixup";;
		*.pl)  cleanups="$cleanups | perl $FIXUPDIR/$fixup";;
		*.awk) cleanups="$cleanups | awk -f $FIXUPDIR/$fixup";;
		    *) echo Unknown fixup type: $fixup;;
            esac
	elif [ -f $FIXUPDIR2/$fixup ]
	then
	    case $fixup in
		*.sed) cleanups="$cleanups | sed -f $FIXUPDIR2/$fixup";;
		*.pl)  cleanups="$cleanups | perl $FIXUPDIR2/$fixup";;
		*.awk) cleanups="$cleanups | awk -f $FIXUPDIR2/$fixup";;
		    *) echo Unknown fixup type: $fixup;;
            esac
	else
	    echo Fixup $fixup not found.
	    success="missing fixup"
	    return
        fi
    done

    fixedoutput=OUTPUT${KLIPS_MODULE}/${prefix}console-fixed.txt
    rm -f $fixedoutput OUTPUT${KLIPS_MODULE}/${prefix}console.diff
    $CONSOLEDIFFDEBUG && echo Cleanups is $cleanups
    eval $cleanups >$fixedoutput

    # stick terminating newline in for fun.
    echo >>$fixedoutput

    if diff -N -u -w -b -B $ref $fixedoutput >OUTPUT${KLIPS_MODULE}/${prefix}console.diff
    then
	echo "${prefix}Console output matched"
    else
	echo "${prefix}Console output differed"

	case "$success" in
	true)	failnum=2 ;;
	esac

	success=false
    fi
}

export_variables() {
    # Xhost script takes things from the environment.
    for host in $XHOST_LIST
    do
       verboseecho "Processing exports for $host"
       verboseecho
       lhost=`echo $host | tr 'A-Z' 'a-z'`

       local startvar
       startvar=${host}_START
       if [ -z "${!startvar-}" ]
       then
            local startdir
	    local starthost
	    starthost=${host}HOST
	    startdir=$POOLSPACE/${!starthost}
            verboseecho "Creating start variable: ${startdir} ${starthost}"
            eval ${host}_START=$startdir/start.sh
       fi
       export ${host}_START

       eval "REF${KERNVER}_${host}_CONSOLE_RAW=OUTPUT${KLIPS_MODULE}/${KERNVER}${lhost}console.txt"
       export REF${KERNVER}_${host}_CONSOLE_RAW
       export ${host}_INIT_SCRIPT
       export ${host}_RUN_SCRIPT
       export ${host}_RUN2_SCRIPT
       export ${host}_RUN3_SCRIPT
       export ${host}_RUN4_SCRIPT
       export ${host}_RUN5_SCRIPT
       export ${host}_FINAL_SCRIPT
    done

    for net in $XNET_LIST
    do
      NET=`echo $net | tr a-z A-Z`
	export ${NET}_PLAY
	export ${NET}_REC
	export ${NET}_ARPREPLY
    done

    export NORTH_PLAY
    export SOUTH_PLAY
    export XHOST_LIST
    export XNET_LIST
    UNTI_SRCDIR=${UNSTRUNG_SRCDIR}
    export UNSTRUNG_SRCDIR
    export UNTI_SRCDIR
    export TEST_PURPOSE
}

# this is called to set additional variables that depend upon testparams.sh
prerunsetup() {
    REPORT_NAME=${TESTNAME}

    # export variables that are common.
    export PACKETRATE KERNVER

    if [ -z "${XHOST_LIST-}" ]
	then
        XHOST_LIST=${UNSTRUNG_HOSTS}
    fi
    
    if [ -z "${XNET_LIST-}" ]
    then
	XNET_LIST=${UNSTRUNG_NETS}
    fi

    export XHOST_LIST

    summarize_results
}


setup_additional_hosts() {

    if [ -n "${ADDITIONAL_HOSTS-}" ]
    then
        SEP=""
	HOSTLIST=""
	for host in ${ADDITIONAL_HOSTS}
	do
	    HOSTLIST="${HOSTLIST}${SEP}${host}=${POOLSPACE}/${host}/start.sh"
	    SEP=","
	done

	echo "-H ${HOSTLIST}"
    fi
}

#
# use this function to run some script on each reference output script.
#
# Start this from testing/*, listing all the test names (directories),
# space-separated. The script to run is the first argument.
#
# The script will be provided with three arguments -
#    1) the name of the test
#    2) the name of the console# which is either "", east or west
#    3) and the file where the reference console should be placed.
#
# The current working directory is *NOT* changed before the script is ran.
#
foreach_ref_console() {
    script=$1
    shift

    for i in $*
    do
	    echo $i:
	    if [ -d $i ]
	    then
		    (if [ -f $i/testparams.sh ]
		    then
			    . $i/testparams.sh
			    compat_variables;
			    if [ -n "${REF_CONSOLE_OUTPUT-}" ]
			    then
				echo $script $i "" $REF_CONSOLE_OUTPUT
				$script $i "" $REF_CONSOLE_OUTPUT
			    fi
			    if [ -n "${REF_EAST_CONSOLE_OUTPUT-}" ]
			    then
				echo $script $i east $REF_EAST_CONSOLE_OUTPUT
				$script $i east $REF_EAST_CONSOLE_OUTPUT
			    fi
			    if [ -n "${REF_WEST_CONSOLE_OUTPUT-}" ]
			    then
				echo $script $i west $REF_WEST_CONSOLE_OUTPUT
				$script $i west $REF_WEST_CONSOLE_OUTPUT
			    fi
		    fi)
	    fi
    done
}

roguekill() {
    REPORT_NAME="$1"
    local rogue_sighted=""

    if [ -n "${REGRESSRESULTS-}" ]
    then
	rm -f $REGRESSRESULTS/$REPORT_NAME/roguelist.txt
	mkdir -p $REGRESSRESULTS/$REPORT_NAME
    fi

    # search for rogue UML
    local pointless=false
    local firstpass=true
    local other_rogues=""
    verboseecho "UML_BRAND=$UML_BRAND"
    for sig in KILL CONT KILL CONT KILL CONT KILL
    do
	if $pointless
	then
	    break;
	fi
	pointless=true
	for i in `grep -s -l '^'"$POOLSPACE"'/[a-z]*/linux\>' /proc/[1-9]*/cmdline`
	do
	    local pdir=`dirname "$i"`
	    local badpid=`basename $pdir`
	    if [ ! -r $pdir/environ ] || strings $pdir/environ | grep "^UML_BRAND=$UML_BRAND"'$' >/dev/null
	    then
		echo "${sig}ING ROGUE UML: $badpid `tr '\000' ' ' <$pdir/cmdline`"
		if [ -n "${REGRESSRESULTS-}" ]
		then
		   echo "UML pid $pdir went ROGUE" >>$REGRESSRESULTS/$REPORT_NAME/roguelist.txt
		fi

		# the cwd is a good indication of what test was being executed.
		rogue_sighted=" rogue"
		pointless=false
		ls -l $pdir/cwd
		kill -$sig $badpid
	    elif $firstpass
	    then
		other_rogues="$other_rogues $badpid"
	    fi
	done
	# might take some realtime for a kill to work
	if ! $pointless
	then
	    sleep 2
	fi
	firstpass=false
    done
    if [ -n "$other_rogues" ]
    then
	echo "ROGUES without brand $UML_BRAND:"
	ps -f -w -p $other_rogues
    fi
    stat="$stat$rogue_sighted"
}

#
# record results records the status of each test in
#   $REGRESSRESULTS/$REPORT_NAME/status
#
# If the status is negative, then the "OUTPUT${KLIPS_MODULE}" directory of the test is
# copied to $REGRESSRESULTS/$REPORT_NAME/OUTPUT${KLIPS_MODULE} as well.
#
# The file $testname/description.txt if it exists is copied as well.
#
# If $REGRESSRESULTS is not set, then nothing is done.
#
# See testing/utils/regress-summarizeresults.pl for a tool to build a nice
# report from these files.
#
# See testing/utils/regress-nightly.sh and regress-stage2.sh for code
# that sets up $REGRESSRESULTS.
#
# usage: recordresults testname testtype status REPORTNAME copybadresults
#
recordresults() {
    local testname="$1"
    local testexpect="$2"
    local status="$3"
    local REPORT_NAME="$4"
    local copybadresults="$5"

    if [ -z "$copybadresults" ]
    then
	copybadresults=true
    fi

    ECHO=${ECHO-echo}

    export REGRESSRESULTS
    roguekill $REPORT_NAME

    if [ -n "${REGRESSRESULTS-}" ]
    then
	rm -rf $REGRESSRESULTS/$REPORT_NAME
	mkdir -p $REGRESSRESULTS/$REPORT_NAME
	console=false
	packet=false

	# if there was a core file, add that to status
	cores=`( lookforcore $testname )`
	if [ ! -z "$cores" ]
	then
	    status="$status core"
	fi

	# if there was a rogue, add that to status
	if [ -f $REGRESSRESULTS/$REPORT_NAME/status/roguelist.txt ]
	then
	    status="$status rogue"
	fi

	# note that 0/1 is shell sense.
	case "$status" in
	    0) success=true;;
	    1) success=false; console=true;;
	    2) success=false; console=false; packet=true;;
	    99) success="missing 99"; console=false; packet=false;;
	    true)  success=true;;
	    false) sucesss=false;;
	    succeed) success=true;;
	    fail)  success=false;;
	    yes)   success=true;;
	    no)    success=false;;
	    skipped) success=skipped;;
	    missing) success=missing;;
	    *)	success=false;;
	esac

	${ECHO} "Recording "'"'"$success: $status"'"'" to $REGRESSRESULTS/$REPORT_NAME/status"
	echo "$success: $status" >$REGRESSRESULTS/$REPORT_NAME/status
	echo console=$console >>$REGRESSRESULTS/$REPORT_NAME/status
	echo packet=$packet   >>$REGRESSRESULTS/$REPORT_NAME/status

	echo "$testexpect" >$REGRESSRESULTS/$REPORT_NAME/expected

	if [ -f $testname/description.txt ]
	then
	    cp $testname/description.txt $REGRESSRESULTS/$REPORT_NAME
	fi


	# the following is in a subprocess to protect against certain
	# testparams.sh which exit!
	(
	    if [ -r "$testdir/testparams.sh" ]
	    then
		. "$testdir/testparams.sh"
	    fi

	    case "${TEST_PURPOSE}" in
	    regress) echo ${TEST_PROB_REPORT} >$REGRESSRESULTS/$REPORT_NAME/regress.txt;;
	       goal) echo ${TEST_GOAL_ITEM}   >$REGRESSRESULTS/$REPORT_NAME/goal.txt;;
	    exploit) echo ${TEST_EXPLOIT_URL} >$REGRESSRESULTS/$REPORT_NAME/exploit.txt;;
		  *) echo "unknown TEST_PURPOSE (${TEST_PURPOSE})" ;;
	    esac
	)

	if $copybadresults
	then
	    case "$success" in
	    false)
		# this code is run only when success is false, so that we have
		# a record of why the test failed. If it succeeded, then the
		# possibly volumnous output is not interesting.
		# 
		# NOTE: ${KLIPS_MODULE} is part of $REPORT_NAME
		rm -rf $REGRESSRESULTS/$REPORT_NAME/OUTPUT
		mkdir -p $REGRESSRESULTS/$REPORT_NAME/OUTPUT
		tar -C $testname/OUTPUT${KLIPS_MODULE} -c -f - . | (cd $REGRESSRESULTS/$REPORT_NAME/OUTPUT && tar xf - )
		;;
	    esac
	fi
    fi

    case "$status" in
    0)	echo '*******  PASSED '$REPORT_NAME' ********' ;;
    skipped)  echo '*******  SKIPPED '$REPORT_NAME' ********' ;;
    *)  echo '*******  FAILED '$REPORT_NAME' ********' ;;
    esac
}

#
#    pcap_filter west   $REF_OUTPUT $WESTOUTPUT $REF_WEST_FILTER
#
pcap_filter() {

    HOST=
    OUTPUT=
    FILTER=
    REF_OUTPUT=

    #echo PCAP_FILTER $@
    HOST=$1
    REF_OUTPUT=$2
    OUTPUT=$3
    FILTER=$4

    if [ -z "$FILTER" ]
    then
	FILTER=cat
    fi

    PATH=$PATH:${FIXUPDIR}
    export PATH

    if [ -n "${OUTPUT-}" ]
    then
	rm -f OUTPUT/${OUTPUT}.txt
	verboseecho "$TCPDUMP -n -t $TCPDUMPFLAGS | $FILTER > OUTPUT/${OUTPUT}.txt"
	eval "$TCPDUMP -n -t $TCPDUMPFLAGS -r OUTPUT/$OUTPUT.pcap | $FILTER >|OUTPUT/${OUTPUT}.txt"

	rm -f OUTPUT/$OUTPUT.diff
	if diff -u -w -b -B $REF_OUTPUT OUTPUT/$OUTPUT.txt >OUTPUT/$OUTPUT.diff
	then
	    printf "%-8s side output matched\n" $HOST
	else
	    printf "%-8s side output differed\n" $HOST
	    success=false
	fi
    fi
}

compareoutputs() {
    for net in $XNET_LIST
    do
      NET=`echo $net | tr a-z A-Z`
      REF_OUTPUT=REF_${NET}_OUTPUT
      OUTPUT=${net}
      REF_FILTER=REF_${NET}_FILTER
      verboseecho "Processing ${net} with var ${REF_OUTPUT}/${OUTPUT}/${REF_FILTER}"
      verboseecho "Variables are: ref: '${!REF_OUTPUT-}' output: '${OUTPUT}' filter: '${!REF_FILTER-}'"

      if [ -n "${!REF_OUTPUT}" ]; then
          pcap_filter $net "${!REF_OUTPUT-}" "${OUTPUT}" "${!REF_FILTER-}"
      fi
    done

    for host in $XHOST_LIST
    do
       local consoleref
       consoleref=REF${KERNVER}_${host}_CONSOLE_OUTPUT
       lhost=`echo $host | tr 'A-Z' 'a-z'`

	if [ -n "${!consoleref-}" ]
	then
	    consolediff ${KERNVER}$lhost OUTPUT/${KERNVER}${lhost}console.txt ${!consoleref}
	fi
    done
}

###################################
#
#  test type: skiptest - do nothing right now.
#
###################################
skiptest() {
    testdir=$1
    testexpect=$2

    export TEST_PURPOSE=regress

    UML_BRAND=0 recordresults $testdir "$testexpect" skipped $testdir ""
}

###################################
#
#  test type: umlXhost - a test with many hosts under control
#
###################################

do_umlX_test() {

    prerunsetup

    success=true
    failnum=1

    # these are network names
    EASTOUTPUT=''
    WESTOUTPUT=''
    PUBOUTPUT=''

    EXP2_ARGS=''

    for net in $XNET_LIST
    do
      NET=`echo $net | tr a-z A-Z`
      verboseecho Setting ${NET}_REC=OUTPUT/${net}.pcap
      eval ${NET}_REC=OUTPUT/${net}.pcap
    done

    export_variables

    if [ -n "${NETJIG_EXTRA-}" ]
    then
	EXP2_ARGS="$EXP2_ARGS -N $NETJIG_EXTRA"
    fi

    EXP2_ARGS="$EXP2_ARGS "`setup_additional_hosts`

    cmd="expect -f $UTILS/Xhost-test.tcl -- -n $NJ $EXP2_ARGS "
    $NETJIGDEBUG && echo NETJIGCMD: $cmd
    eval $cmd

    compareoutputs;


    case "$success" in
    true)	exit 0 ;;
    *)		exit 1 ;;
    esac
}


# the test entry point itself
pkttest() {
    testdir=$1
    testexpect=$2

    echo '***** UML 3HOST RUNNING' $testdir '*******'

    export UML_BRAND="$$"
    ( preptest $testdir pkttest && do_umlX_test )
    stat=$?

    recordresults $testdir "$testexpect" "$stat" $testdir ""
}


###################################
#
#  test type: unittest
#
# testparams.sh should specify a script to be run as $TESTSCRIPT
#          REF_CONSOLE_OUTPUT= name of reference output
#    
# The script will be started with:
#          ROOTDIR=    set to root of source code.
#          OBJDIRTOP=  set to location of object files
# 
#
# testparams.sh should set PROGRAMS= to a list of subdirs of programs/
#                that must be built before using the test. This allows
#                additional modules to be built.
#
# If there is a Makefile in the subdir, it will be invoked as
# "make checkprograms". It will have the above variables as well,
# and make get the build environment with 
#    include ${ROOTDIR}/programs/Makefile.program
#
# The stdout of the script will be set to an output file, which will then
# be sanitized using the normal set of fixup scripts.
#          
#
###################################

do_unittest() {

    export ROOTDIR=${UNTI_SRCDIR}
    eval `(cd $ROOTDIR; make --no-print-directory env )`
    failnum=1

    if [ ! -x "$TESTSCRIPT" ]; then echo "TESTSCRIPT=$TESTSCRIPT is not executable"; exit 41; fi

    echo "BUILDING DEPENDANCIES"
    (cd ${ROOTDIR}/programs;
     for program in ${PROGRAMS}
     do
	if [ -d $program ]; then (cd $program && make programs checkprograms ); fi
     done)

    echo "BUILDING TEST CASE"
    # if there is a makefile, run it and bail if fails
    if [ -f Makefile ]; then
	if make checkprograms; then
	    :
	else
	    exit 1;
	fi
    fi

    # make sure we get all core dumps!
    ulimit -c unlimited
    export OBJDIRTOP

    OUTDIR=${OBJDIRTOP}/testing/${TESTSUBDIR}/${TESTNAME}
    mkdir -p ${OUTDIR}
    rm -f OUTPUT; ln -f -s ${OUTDIR} OUTPUT

    echo "RUNNING $TESTSCRIPT"
    ./$TESTSCRIPT >${OUTDIR}/console.txt
    echo "DONE    $TESTSCRIPT"

    stat=$?
    echo Exit code $stat
    if [ $stat -gt 128 ]
    then
	stat="$stat core"
    else
        consolediff "" OUTPUT/console.txt $REF_CONSOLE_OUTPUT
	case "$success" in
	true)	exit 0 ;;
	*)	exit $failnum ;;
	esac
    fi
}

unittest() {
    testcase=$1
    testexpect=$2

    echo '**** make unittest RUNNING '$testcase' ****'

    echo Running $testobj
    ( preptest $testcase unittest false && do_unittest )
    stat=$?

    TEST_PURPOSE=regress recordresults $testcase "$testexpect" "$stat" $testcase false
}





