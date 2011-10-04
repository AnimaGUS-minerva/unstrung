#!/bin/sh
#
# 
# $Id: make-uml.sh,v 1.51 2005/11/21 08:44:57 mcr Exp $
#

# show me
#set -x

# fail if any command fails
set -e

case $# in
    1) UNSTRUNG_SRCDIR=$1; shift;;
esac

CC=${CC-cc}

if [ `id -u` = 0 ]
then
    echo Do not run this as root.
    exit
fi

# we always use OBJ directories for UML builds.
export USE_OBJDIR=true

# include this dir, in particular so that we can get the local "touch"
# program.
export PATH=$UNSTRUNG_SRCDIR/testing/utils:$PATH 


#
# configuration for this file has moved to $UNSTRUNG_SRCDIR/umlsetup.sh
# By default, that file does not exist. A sample is at umlsetup-sample.sh
# in this directory. Copy it to $UNSTRUNG_SRCDIR and edit it.
#
UNSTRUNG_SRCDIR=${UNSTRUNG_SRCDIR-../..}
if [ ! -f ${UNSTRUNG_SRCDIR}/umlsetup.sh ]
then
    echo No umlsetup.sh. Please read instructions in doc/umltesting.html and testing/utils/umlsetup-sample.sh.
    exit 1
fi

. ${UNSTRUNG_SRCDIR}/umlsetup.sh
. ${UNSTRUNG_SRCDIR}/testing/utils/uml-functions.sh

KERNVERSION=2.6;

echo Setting up for kernel KERNVERSION=$KERNVERSION


# make absolute so that we can reference it from POOLSPACE
UNSTRUNG_SRCDIR=`cd $UNSTRUNG_SRCDIR && pwd`;export UNSTRUNG_SRCDIR

# what this script does is create some Makefile
#  (if they do not already exist)
# that will copy everything where it needs to go.

if [ ! -d $UNSTRUNG_SRCDIR/testing ]
then
    echo Where is ${UNSTRUNG_SRCDIR}/testing '????'
    exit 1
fi
TESTINGROOT=$UNSTRUNG_SRCDIR/testing

# more defaults
NONINTCONFIG=oldconfig

# hack for version specific stuff
UMLVERSION=`basename $UMLPATCH .bz2 | sed -e 's/uml-patch-//'`
EXTRAPATCH=${TESTINGROOT}/kernelconfigs/extras.$UMLVERSION.patch

# dig the kernel revision out.
KERNEL_MAJ_VERSION=`${UNSTRUNG_SRCDIR}/testing/utils/kernelversion-short $KERNPOOL/Makefile`


echo -n Looking for Extra patch at $EXTRAPATCH..
if [ -f "${EXTRAPATCH}" ]
then
    echo found it.
else
    echo none.
    EXTRAPATCH=
fi

mkdir -p $POOLSPACE
if [ ! -d ${UNSTRUNG_SRCDIR}/UMLPOOL/. ]; then ln -s $POOLSPACE ${UNSTRUNG_SRCDIR}/UMLPOOL; fi

UMLMAKE=$POOLSPACE/Makefile
NOW=`date`
USER=${USER-`id -un`}
echo '#' built by $0 on $NOW by $USER >|$UMLMAKE
echo '#' >>$UMLMAKE

# okay, copy the kernel, apply the UML patches, and build a plain kernel.
UMLPLAIN=$POOLSPACE/plain
mkdir -p $UMLPLAIN

# now, setup up root dir
NEED_plain=false

# go through each regular host and see what kernel to use, and
# see if we have to build the local plain kernel.
for host in $UNSTRUNG_HOSTS
do
    kernelvar=UML_plain_KERNEL
    UMLKERNEL=${!kernelvar}
    if [ -z "${UMLKERNEL}" ]
    then
	kernelvar=UML_${host}_KERNEL
	UMLKERNEL=${!kernelvar}
	if [ -z "${UMLKERNEL}" ]
	then
	    # must need stock kernel.
	    UMLKERNEL=${UMLPLAIN}/linux
	    NEED_plain=true
	fi
    fi
    echo Using kernel: $UMLKERNEL for $host

    setup_host_make $host $UMLKERNEL regular >>$UMLMAKE
done

# build a plain kernel if we need it!
if $NEED_plain && [ ! -x $UMLPLAIN/linux ]
then
    cd $UMLPLAIN

    lndirkerndirnogit $KERNPOOL .

    applypatches

    echo Copying kernel config ${TESTINGROOT}/kernelconfigs/umlplain.config 
    rm -f .config
    cp ${TESTINGROOT}/kernelconfigs/umlplain.config .config
    
    (make CC=${CC} ARCH=um $NONINTCONFIG && make CC=${CC} ARCH=um dep && make ARCH=um CC=${CC} linux ) || exit 1 </dev/null 
fi

BUILD_MODULES=${BUILD_MODULES-true}
if $NEED_plain
then
    :
else
    BUILD_MODULES=false
fi
    
setup_make $BUILD_MODULES >>$UMLMAKE

# now, execute the Makefile that we have created!
cd $POOLSPACE && make $REGULARHOSTS 

# now, copy the kernel, apply the UML patches.
# then, make FreeSWAN patches as well.
#
UMLSWAN=$POOLSPACE/pandora

# we could copy the UMLPLAIN to make this tree. This would be faster, as we
# already built most everything. We could also just use a FreeSWAN-enabled
# kernel on sunrise/sunset. We avoid this as we actually want them to always
# work.

# where to install FreeSWAN tools
DESTDIR=$POOLSPACE/root

# do not generate .depend by default
KERNDEP=''

mkdir -p $UMLSWAN

# now, setup up root dir
NEED_swan=false

# go through each regular host and see what kernel to use, and
# see if we have to build the local plain kernel.
for host in $SPECIAL_HOSTS
do
    kernelvar=UML_swan${KERNVER}_KERNEL
    UMLKERNEL=${!kernelvar}
    if [ -z "${UMLKERNEL}" ]
    then
	kernelvar=UML_${host}_KERNEL
	UMLKERNEL=${!kernelvar}
	if [ -z "${UMLKERNEL}" ]
	then
	    # must need stock kernel.
	    UMLKERNEL=${UMLSWAN}/linux
	    NEED_swan=true
	fi
    fi
    echo Using kernel: $UMLKERNEL for $host

    setup_host_make $host $UMLKERNEL pandora ${KERNVER} $NEED_swan >>$UMLMAKE
done

if $NEED_swan && [ ! -x $UMLSWAN/linux ]
then
    cd $UMLSWAN
    lndirkerndirnogit $KERNPOOL .

    applypatches
    
    # copy the config file
    rm -f .config
    cp ${TESTINGROOT}/kernelconfigs/umlpandora${KERNVER}.config .config

    # nuke final executable here since we will do FreeSWAN in a moment.
    rm -f linux .depend
    KERNDEP=dep

    grep CONFIG_KLIPS $UMLSWAN/.config || exit 1
fi

if $NEED_swan && [ ! -x $UMLSWAN/linux ]
then
    cd $UNSTRUNG_SRCDIR || exit 1
 
    make KERNMAKEOPTS='ARCH=um' KERNELSRC=$UMLSWAN KERNCLEAN='' KERNDEP=$KERNDEP KERNEL=linux DESTDIR=$DESTDIR verset kpatch rcf kernel || exit 1 </dev/null 

    # mark it as read-only, so that we don't edit the wrong files by mistake!
    find $UMLSWAN/net/ipsec $UMLSWAN/include/openswan -name '*.[ch]' -type f -print | xargs chmod a-w
fi

cd $UNSTRUNG_SRCDIR || exit 1

make programs

# now, execute the Makefile that we have created!
cd $POOLSPACE && make $UNSTRUNG_HOSTS 

