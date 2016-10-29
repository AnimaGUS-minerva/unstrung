#!/bin/sh

set -e
mkdir -p $HOME/stuff
BUILDTOP=$(cd $HOME/stuff; pwd)
ARCH=$(arch)

case $ARCH in
    x86_64) HOST=x86_64;;
    i386)   HOST=i386;;
esac

set -x
rm -f ${BUILDTOP}/host
ln -f -s ${BUILDTOP}/${HOST} ${BUILDTOP}/host
mkdir -p ${BUILDTOP}/${HOST}

if [ ! -d $BUILDTOP/include/mbedtls ]; then
    if [ ! -d $BUILDTOP/mbedtls ]; then (cd ${BUILDTOP} && git clone -b mcr_add_otherName https://github.com/mcr/mbedtls.git ); fi
    set -x
    (cd ${BUILDTOP} && rm -rf host/mbedtls && mkdir -p host/mbedtls && cd host/mbedtls && cmake -DCMAKE_INSTALL_PREFIX=$BUILDTOP ../../mbedtls && make CFLAGS='--coverage -g3 -O0' && make install)
fi

LIBPCAP=${BUILDTOP}/host/libpcap-1.8.1/libpcap.a
if [ ! -x $BUILDTOP/host/tcpdump-4.8.1/tcpdump ]
then
    if [ ! -d ${BUILDTOP}/libpcap ]; then (cd ${BUILDTOP} && git clone -b libpcap-1.8.1 https://github.com/the-tcpdump-group/libpcap.git ); fi
    if [ ! -d ${BUILDTOP}/tcpdump ]; then (cd ${BUILDTOP} && git clone -b tcpdump-4.8.1 https://github.com/the-tcpdump-group/tcpdump.git ); fi
    case $ARCH in
        x86_64) HOST=x86_64;;

        i386)   HOST=i386;;
    esac

    if [ ! -d "${BUILDTOP}/x86_64/tcpdump/." ]; then
        (cd ${BUILDTOP} && mkdir -p x86_64/libpcap-1.8.1 && cd x86_64/libpcap-1.8.1 && CFLAGS=-m64 ../../libpcap/configure --prefix=$HOME/stuff --target=x86_64-linux-gnu && make LDFLAGS=-m64 CFLAGS=-m64)
        (cd ${BUILDTOP} && ln -f -s x86_64/libpcap-1.8.1 libpcap && mkdir -p x86_64/tcpdump-4.8.1 && cd x86_64/tcpdump-4.8.1 && CFLAGS=-m64 ../../tcpdump/configure --prefix=$HOME/stuff --target=x86_64-linux-gnu && make LDFLAGS=-m64 CFLAGS=-m64)
        (cd ${BUILDTOP}/x86_64 && ln -s -f tcpdump-4.8.1 tcpdump )
    fi

    if [ ! -d "${BUILDTOP}/i386/tcpdump/." ]; then
        (cd ${BUILDTOP} && mkdir -p i386/libpcap-1.8.1 && cd i386/libpcap-1.8.1 && CFLAGS=-m32 ../../libpcap/configure --prefix=$HOME/stuff --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
        (cd ${BUILDTOP} && ln -f -s i386/libpcap-1.8.1 libpcap && mkdir -p i386/tcpdump-4.8.1 && cd i386/tcpdump-4.8.1 && CFLAGS=-m32 ../../tcpdump/configure --prefix=$HOME/stuff --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
        (cd ${BUILDTOP}/i386 && ln -s -f tcpdump-4.8.0 tcpdump )
    fi

    (cd $BUILDTOP/host/libpcap-1.8.1 && make install)
fi
LIBPCAP_HOST_DIR=${BUILDTOP}/${HOST}/libpcap-1.8.1
LIBPCAP_HOST=${LIBPCAP_HOST_DIR}/libpcap.a
LIBPCAP=${LIBPCAP_HOST}
TCPDUMP_HOST_DIR=${BUILDTOP}/${HOST}/tcpdump-4.8.1

if [ ! -f $BUILDTOP/include/cobr.h ]
then
    if [ ! -d ${BUILDTOP}/libcbor ]; then ( cd $BUILDTOP && git clone https://github.com/mcr/libcbor.git) ; fi

    case $ARCH in
        x86_64) HOST=x86_64;;
        i386)   HOST=i386;;
    esac

    (cd ${BUILDTOP} && mkdir -p ${HOST}/libcbor && cd ${HOST}/libcbor && cmake ../../libcbor -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP} && make && make install)



    if [ ! -d "${BUILDTOP}/x86_64/libcbor" ]; then
        (cd ${BUILDTOP} && mkdir -p x86_64/libcbor && cd x86_64/libcbor && cmake ../../libcbor -DCMAKE_C_FLAGS_DEBUG=-m64 -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP}/x86_64 && make && make install)
    fi

    if [ ! -d "${BUILDTOP}/i386/libcbor" ]; then
        (cd ${BUILDTOP} && mkdir -p i386/libcbor && cd i386/libcbor && cmake ../../libcbor -DCMAKE_C_FLAGS_DEBUG=-m32 -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP}/i386 && make && make install)
    fi
fi

echo LIBPCAP=${LIBPCAP} -lpthread -ldbus-1 >Makefile.local
echo LIBPCAP_HOST=${LIBPCAP_HOST} -lpthread -lusb-1.0 -ldbus-1 >>Makefile.local
echo LIBPCAPINC=-I${LIBPCAP_HOST_DIR}                >>Makefile.local
echo TCPDUMP=${TCPDUMP_HOST_DIR}/tcpdump             >>Makefile.local
echo MBEDTLSH=-I${BUILDTOP}/include                  >>Makefile.local
echo MBEDTLSLIB=${BUILDTOP}/lib                      >>Makefile.local
echo NETDISSECTLIB=${LIBPCAP_HOST_DIR}/libnetdissect.a >>Makefile.local
echo NETDISSECTH=-DNETDISSECT -I${BUILDTOP}/include -I${TCPDUMP_HOST_DIR} -I${BUILDTOP}/tcpdump >>Makefile.local
echo CBOR_LIB=${BUILDTOP}/${HOST}/libcbor/src/libcbor.a      >>Makefile.local
echo CBOR_INCLUDE=-I${BUILDTOP}/include -Drestrict=__restrict__   >>Makefile.local
echo export ARCH LIBPCAP LIBPCAP_HOST LIBPCAPINC TCPDUMP NETDISSECTLIB NETDISSECTH >>Makefile.local
