#!/bin/sh

set -e
mkdir -p $HOME/stuff
BUILDTOP=$(cd $HOME/stuff; pwd)
ARCH=$(arch)

if [ ! -d $BUILDTOP/include/mbedtls ]; then
    if [ ! -d $BUILDTOP/mbedtls ]; then (cd ${BUILDTOP} && git clone -b mcr_add_otherName https://github.com/mcr/mbedtls.git ); fi
    (cd ${BUILDTOP} && rm -rf host/mbedtls && mkdir -p host/mbedtls && cd host/mbedtls && cmake -DCMAKE_INSTALL_PREFIX=$BUILDTOP ../../mbedtls && make && make install)
fi

LIBPCAP=${BUILDTOP}/host/libpcap-1.8.0/libpcap.a
if [ ! -x $BUILDTOP/host/tcpdump-4.8.0/tcpdump ]
then
    if [ ! -d libpcap ]; then (cd ${BUILDTOP} && git clone https://github.com/mcr/libpcap.git ); fi
    if [ ! -d tcpdump ]; then (cd ${BUILDTOP} && git clone https://github.com/mcr/tcpdump.git ); fi
    case $ARCH in
        x86_64) HOST=x86_64;;
        i386)   HOST=i386;;
    esac

    (cd ${BUILDTOP} && mkdir -p ${HOST}/libpcap-1.8.0 && ln -s ${HOST} host && cd ${HOST}/libpcap-1.8.0 && ../../libpcap/configure --prefix=${BUILDTOP} && make && make install)
    (cd ${BUILDTOP} && mkdir -p ${HOST}/tcpdump-4.8.0 && ln -s ${HOST} host && cd ${HOST}/tcpdump-4.8.0 && ../../tcpdump/configure --prefix=${BUILDTOP} && make && make install)

    if [ ! -d "x86_64" ]; then
        (cd ${BUILDTOP} && mkdir -p x86/libpcap-1.8.0 && cd x86/libpcap-1.8.0 && CFLAGS=-m64 ../../libpcap/configure --target=x86_64-linux-gnu && make LDFLAGS=-m64 CFLAGS=-m64)
        (cd ${BUILDTOP} && ln -s x86/libpcap-1.8.0 libpcap && mkdir -p x86/tcpdump-4.8.0 && cd x86/tcpdump-4.8.0 && CFLAGS=-m64 ../../libpcap/configure --target=x86_64-linux-gnu && make LDFLAGS=-m64 CFLAGS=-m64)
    fi

    if [ ! -d "i386" ]; then
        (cd ${BUILDTOP} && mkdir -p i386/libpcap-1.8.0 && cd i386/libpcap-1.8.0 && CFLAGS=-m64 ../../libpcap/configure --target=x86_64-linux-gnu && make LDFLAGS=-m64 CFLAGS=-m64)
        (cd ${BUILDTOP} && ln -s i386/libpcap-1.8.0 libpcap && mkdir -p i386/tcpdump-4.8.0 && cd i386/tcpdump-4.8.0 && CFLAGS=-m32 ../../tcpdump/configure --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
    fi
fi

if [ ! -f $BUILDTOP/include/cobr.h ]
then
    if [ ! -d libcbor ]; then git clone https://github.com/mcr/libcbor.git ; fi

    case $ARCH in
        x86_64) HOST=x86_64;;
        i386)   HOST=i386;;
    esac

    (cd ${BUILDTOP} && mkdir -p ${HOST}/libcbor && cd ${HOST}/libcbor && cmake ../../libcbor -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP} && make && make install)



    if [ ! -d "x86_64/libcbor" ]; then
        (cd ${BUILDTOP} && mkdir -p x86_64/libcbor && cd x86_64/libcbor && cmake ../../libcbor -DCMAKE_C_FLAGS_DEBUG=-m64 -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP}/x86_64 && make && make install)
    fi

    if [ ! -d "i386/libcbor" ]; then
        (cd ${BUILDTOP} && mkdir -p i386/libcbor && cd i386/libcbor && cmake ../../libcbor -DCMAKE_C_FLAGS_DEBUG=-m32 -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP}/i386 && make && make install)
    fi
fi

LIBPCAP=${BUILDTOP}/host/libpcap-1.8.0/libpcap.a

echo LIBPCAP=${LIBPCAP} -lpthread -ldbus-1 >Makefile.local
echo LIBPCAP_HOST=${BUILDTOP}/host/libpcap-1.8.0/libpcap.a >>Makefile.local
echo LIBPCAPINC=-I${BUILDTOP}/include                >>Makefile.local
echo TCPDUMP=${BUILDTOP}/host/tcpdump-4.8.0/tcpdump  >>Makefile.local
echo MBEDTLSH=-I${BUILDTOP}/include                  >>Makefile.local
echo MBEDTLSLIB=${BUILDTOP}/lib                      >>Makefile.local
echo NETDISSECTLIB=${BUILDTOP}/host/tcpdump-4.8.0/libnetdissect.a >>Makefile.local
echo NETDISSECTH=-DNETDISSECT -I${BUILDTOP}/include -I${BUILDTOP}/host/tcpdump-4.8.0/ -I${BUILDTOP}/tcpdump >>Makefile.local
echo CBOR_LIB=${BUILDTOP}/${HOST}/lib/libcbor.a      >>Makefile.local
echo CBOR_INCLUDE=-I${BUILDTOP}/include -Drestrict=__restrict__   >>Makefile.local
echo export ARCH LIBPCAP LIBPCAP_HOST LIBPCAPINC TCPDUMP NETDISSECTLIB NETDISSECTH >>Makefile.local
