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
    (cd ${BUILDTOP} && mkdir -p host/libpcap-1.8.0 && cd host/libpcap-1.8.0 && ../../libpcap/configure --prefix=${BUILDTOP} && make && make install)
    (cd ${BUILDTOP} && mkdir -p host/tcpdump-4.8.0 && cd host/tcpdump-4.8.0 && ../../tcpdump/configure --prefix=${BUILDTOP} && make && make install)

    if [ "$ARCH" = "x86_64" ]; then
        (cd ${BUILDTOP} && mkdir -p x86/libpcap-1.8.0 && cd x86/libpcap-1.8.0 && CFLAGS=-m32 ../../libpcap/configure --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
        (cd ${BUILDTOP} && ln -s x86/libpcap-1.8.0 libpcap && mkdir -p x86/tcpdump-4.8.0 && cd x86/tcpdump-4.8.0 && CFLAGS=-m32 ../../tcpdump/configure --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
        LIBPCAP=${BUILDTOP}/x86/libpcap-1.8.0/libpcap.a
    fi
fi

if [ "$ARCH" = "x86_64" ]; then
    LIBPCAP=${BUILDTOP}/x86/libpcap-1.8.0/libpcap.a
fi

echo LIBPCAP=${LIBPCAP} >Makefile.local
echo LIBPCAP_HOST=${BUILDTOP}/host/libpcap-1.8.0/libpcap.a >>Makefile.local
echo LIBPCAPINC=-I${BUILDTOP}/include                >>Makefile.local
echo TCPDUMP=${BUILDTOP}/host/tcpdump-4.8.0/tcpdump  >>Makefile.local
echo MBEDTLSH=-I${BUILDTOP}/include                  >>Makefile.local
echo MBEDTLSLIB=${BUILDTOP}/lib                      >>Makefile.local
echo NETDISSECTLIB=${BUILDTOP}/host/tcpdump-4.8.0/libnetdissect.a >>Makefile.local
echo NETDISSECTH=-DNETDISSECT -I${BUILDTOP}/include -I${BUILDTOP}/host/tcpdump-4.8.0/ -I${BUILDTOP}/tcpdump >>Makefile.local
echo export ARCH LIBPCAP LIBPCAP_HOST LIBPCAPINC TCPDUMP NETDISSECTLIB NETDISSECTH >>Makefile.local
