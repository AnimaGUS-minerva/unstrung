#!/bin/sh

BUILDTOP=`pwd`/stuff
USE_WOLFSSL=false
USE_MBEDTLS=false

touch Makefile.local

if ${USE_MBEDTLS}; then
    mkdir -p $BUILDTOP
    cd $BUILDTOP
    [ -d mbedtls ] || git clone -b mcr_add_otherName https://github.com/mcr/mbedtls.git
    (cd mbedtls && cmake -DCMAKE_INSTALL_PREFIX=$BUILDTOP . && make && make install )

    if grep MBEDTLS Makefile.local
    then
        :
    else
        echo MBEDTLSH=-I${BUILDTOP}/include          >>Makefile.local
        echo MBEDTLSLIB=${BUILDTOP}/lib              >>Makefile.local
    fi
fi

if ${USE_WOLFSSL}; then
    mkdir -p $BUILDTOP
    cd $BUILDTOP
    [ -d wolfssl ] || git clone https://github.com/wolfSSL/wolfssl.git
    # (cd wolfssl && ./autogen.sh && ./configure --prefix= )

    if grep WOLFSSL Makefile.local
    then
        :
    else
        echo WOLFSSLH=-I/sandel/include/wolfssl       >>Makefile.local
        echo WOLFSSLLIB=-L/sandel/lib -lwolfssl       >>Makefile.local
    fi
fi

if [ ! -f $BUILDTOP/include/cbor.h ]
then
    if [ ! -d ${BUILDTOP}/libcbor ]; then ( cd $BUILDTOP && git clone https://github.com/mcr/libcbor.git) ; fi

    (cd ${BUILDTOP}/libcbor && cmake . -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP} && make && make install)
fi

if grep CBOR_LIB Makefile.local
then
    :
else
    echo CBOR_LIB=${BUILDTOP}/libcbor/src/libcbor.a      >>Makefile.local
    echo CBOR_INCLUDE=-I${BUILDTOP}/include -Drestrict=__restrict__   >>Makefile.local
fi



