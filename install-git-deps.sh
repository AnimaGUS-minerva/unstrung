#!/bin/sh

BUILDTOP=`pwd`/stuff

mkdir -p $BUILDTOP
cd $BUILDTOP
[ -d mbedtls ] || git clone -b mcr_add_otherName https://github.com/mcr/mbedtls.git
(cd mbedtls && cmake -DCMAKE_INSTALL_PREFIX=$BUILDTOP . && make && make install )

if [ ! -f $BUILDTOP/include/cbor.h ]
then
    if [ ! -d ${BUILDTOP}/libcbor ]; then ( cd $BUILDTOP && git clone https://github.com/mcr/libcbor.git) ; fi

    (cd ${BUILDTOP}/libcbor && cmake . -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP} && make && make install)
fi


touch Makefile.local
if grep MBEDTLS Makefile.local
then
    :
else
    echo MBEDTLSH=-I${BUILDTOP}/include          >>Makefile.local
    echo MBEDTLSLIB=${BUILDTOP}/lib              >>Makefile.local
fi

if grep MBEDTLS Makefile.local
then
    :
else
    echo CBOR_LIB=${BUILDTOP}/libcbor/src/libcbor.a      >>Makefile.local
    echo CBOR_INCLUDE=-I${BUILDTOP}/include -Drestrict=__restrict__   >>Makefile.local
fi



