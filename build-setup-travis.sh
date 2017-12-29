#!/bin/sh

set -e
mkdir -p $HOME/stuff
BUILDTOP=$(cd $HOME/stuff; pwd)
ARCH=${ARCH-$(uname -m)}

case $(arch) in
    x86_64) HOST=x86_64;;
    i386)   HOST=i386;;
    armv7l) HOST=armv7l;;
esac

set -x

build64=true
build32=true
buildgen=false
case $1 in
    ONLYARCH=x86_64) build32=false;;
    ONLYARCH=i386)   build64=false;;
    *) buildgen=true; build32=false; build64=false;;
esac

if $build32; then
    echo Building 32-bit.
fi
if $build64; then
    echo Building 64-bit.
fi


rm -f ${BUILDTOP}/host
ln -f -s ${BUILDTOP}/${HOST} ${BUILDTOP}/host
mkdir -p ${BUILDTOP}/${HOST}

    if [ ! -d $BUILDTOP/mbedtls ]; then (cd ${BUILDTOP} && git clone -b mcr_add_otherName https://github.com/mcr/mbedtls.git ); fi

    if $build64 && [ ! -d "${BUILDTOP}/x86_64/mbedtls" ]; then
        (cd ${BUILDTOP} && rm -rf x86_64/mbedtls && mkdir -p x86_64/mbedtls && cd x86_64/mbedtls && cmake -DCMAKE_C_FLAGS:STRING=-m64 -DCMAKE_INSTALL_PREFIX=$BUILDTOP/x86_64 ../../mbedtls && make CFLAGS='-m64 --coverage -g3 -O0' && make install)
    fi

    if $build32 && [ ! -d "${BUILDTOP}/i386/mbedtls" ]; then
        (cd ${BUILDTOP} && rm -rf i386/mbedtls && mkdir -p i386/mbedtls && cd i386/mbedtls && cmake -DCMAKE_C_FLAGS:STRING=-m32 -DCMAKE_INSTALL_PREFIX=$BUILDTOP/i386 ../../mbedtls && make CFLAGS='-m32 --coverage -g3 -O0' && make install)
    fi

    if $buildgen && [ ! -d "${BUILDTOP}/${ARCH}/mbedtls" ]; then
        (cd ${BUILDTOP} && rm -rf ${ARCH}/mbedtls && mkdir -p ${ARCH}/mbedtls && cd ${ARCH}/mbedtls && cmake -DCMAKE_INSTALL_PREFIX=$BUILDTOP/${ARCH} ../../mbedtls && make CFLAGS='-coverage -g3 -O0' && make install)
    fi

LIBPCAP=${BUILDTOP}/host/libpcap-1.8.1/libpcap.a
if [ ! -x $BUILDTOP/host/tcpdump-4.8.1/tcpdump ]
then
    if [ ! -d ${BUILDTOP}/libpcap ]; then (cd ${BUILDTOP} && git clone -b libpcap-1.8.1 https://github.com/the-tcpdump-group/libpcap.git ); fi
    if [ ! -d ${BUILDTOP}/tcpdump ]; then (cd ${BUILDTOP} && git clone -b tcpdump-4.8.1 https://github.com/the-tcpdump-group/tcpdump.git ); fi

    if $buildgen && [ ! -d "${BUILDTOP}/${ARCH}/tcpdump-4.8.1/." ]; then
        (cd ${BUILDTOP} && mkdir -p ${ARCH}/libpcap-1.8.1 && cd ${ARCH}/libpcap-1.8.1 && ../../libpcap/configure --prefix=$HOME/stuff --target=${ARCH}-linux-gnu && make CFLAGS="-fPIC")
        (cd ${BUILDTOP} && ln -f -s ${ARCH}/libpcap-1.8.1 libpcap && mkdir -p ${ARCH}/tcpdump-4.8.1 && cd ${ARCH}/tcpdump-4.8.1 && ../../tcpdump/configure --prefix=$HOME/stuff --target=${ARCH}-linux-gnu && make CFLAGS="-fPIC")
        (cd ${BUILDTOP}/${ARCH} && ln -s -f tcpdump-4.8.1 tcpdump )
    fi

    if $build64 && [ ! -d "${BUILDTOP}/x86_64/tcpdump-4.8.1/." ]; then
        (cd ${BUILDTOP} && mkdir -p x86_64/libpcap-1.8.1 && cd x86_64/libpcap-1.8.1 && CFLAGS=-m64 ../../libpcap/configure --prefix=$HOME/stuff --target=x86_64-linux-gnu && make LDFLAGS=-m64 CFLAGS="-m64 -fPIC")
        (cd ${BUILDTOP} && ln -f -s x86_64/libpcap-1.8.1 libpcap && mkdir -p x86_64/tcpdump-4.8.1 && cd x86_64/tcpdump-4.8.1 && CFLAGS=-m64 ../../tcpdump/configure --prefix=$HOME/stuff --target=x86_64-linux-gnu && make LDFLAGS=-m64 CFLAGS="-m64 -fPIC")
        (cd ${BUILDTOP}/x86_64 && ln -s -f tcpdump-4.8.1 tcpdump )
    fi

    if $build32 && [ ! -d "${BUILDTOP}/i386/tcpdump-4.8.1/." ]; then
        (cd ${BUILDTOP} && mkdir -p i386/libpcap-1.8.1 && cd i386/libpcap-1.8.1 && CFLAGS=-m32 ../../libpcap/configure --prefix=$HOME/stuff --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS="-m32 -fPIC")
        (cd ${BUILDTOP} && ln -f -s i386/libpcap-1.8.1 libpcap && mkdir -p i386/tcpdump-4.8.1 && cd i386/tcpdump-4.8.1 && CFLAGS=-m32 ../../tcpdump/configure --prefix=$HOME/stuff --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS="-m32 -fPIC")
        (cd ${BUILDTOP}/i386 && ln -s -f tcpdump-4.8.1 tcpdump )
    fi

    (cd $BUILDTOP/${HOST}/libpcap-1.8.1 && make install)
fi

LIBPCAP_HOST_DIR=${BUILDTOP}/${HOST}/libpcap-1.8.1
LIBPCAP_HOST=${LIBPCAP_HOST_DIR}/libpcap.a
LIBPCAP_DIR=${BUILDTOP}/${ARCH}/libpcap-1.8.1
LIBPCAP=${LIBPCAP_DIR}/libpcap.a

TCPDUMP_HOST_DIR=${BUILDTOP}/${HOST}/tcpdump-4.8.1
TCPDUMP_HOST=${TCPDUMP_HOST_DIR}/tcpdump
TCPDUMP_DIR=${BUILDTOP}/${ARCH}/tcpdump-4.8.1
TCPDUMP=${TCPDUMP_DIR}/tcpdump

if [ ! -f $BUILDTOP/include/cbor.h ]
then
    if [ ! -d ${BUILDTOP}/libcbor ]; then ( cd $BUILDTOP && git clone https://github.com/mcr/libcbor.git) ; fi

    (cd ${BUILDTOP} && mkdir -p ${HOST}/libcbor && cd ${HOST}/libcbor && cmake ../../libcbor -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP} && make && make install)

    if $buildgen && [ ! -d "${BUILDTOP}/${ARCH}/libcbor" ]; then
        (cd ${BUILDTOP} && mkdir -p ${ARCH}/libcbor && cd ${ARCH}/libcbor && cmake ../../libcbor -DCMAKE_C_FLAGS:STRING="-fPIC" -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP}/${ARCH} && make && make install)
    fi

    if $build64 && [ ! -d "${BUILDTOP}/x86_64/libcbor" ]; then
        (cd ${BUILDTOP} && mkdir -p x86_64/libcbor && cd x86_64/libcbor && cmake ../../libcbor -DCMAKE_C_FLAGS:STRING="-m64 -fPIC" -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP}/x86_64 && make && make install)
    fi

    if $build32 && [ ! -d "${BUILDTOP}/i386/libcbor" ]; then
        (cd ${BUILDTOP} && mkdir -p i386/libcbor && cd i386/libcbor && cmake ../../libcbor -DCMAKE_C_FLAGS:STRING="-m32 -fPIC" -DCMAKE_INSTALL_PREFIX:PATH=${BUILDTOP}/i386 && make && make install)
    fi
fi

echo >Makefile.local
echo LIBNL=-lnl-3 -lnl-genl-3 -ldbus-1 -lpthread -lusb-1.0 >>Makefile.local
echo LIBPCAP_HOST_DIR=${LIBPCAP_HOST_DIR}              >>Makefile.local
echo TCPDUMP_HOST_DIR=${TCPDUMP_HOST_DIR}              >>Makefile.local
echo LIBPCAP=${LIBPCAP} '${LIBNL}'                     >>Makefile.local
echo LIBPCAP_HOST=${LIBPCAP_HOST} '${LIBNL}'           >>Makefile.local
echo LIBPCAPINC=-I${LIBPCAP_HOST_DIR}                >>Makefile.local
echo TCPDUMP=${TCPDUMP_HOST_DIR}/tcpdump             >>Makefile.local
echo MBEDTLSH=-I${BUILDTOP}/${ARCH}/include          >>Makefile.local
echo MBEDTLSLIB=${BUILDTOP}/${ARCH}/lib              >>Makefile.local
echo NETDISSECTLIB=${TCPDUMP_HOST_DIR}/libnetdissect.a >>Makefile.local
echo NETDISSECTH=-DNETDISSECT -I${BUILDTOP}/include -I${TCPDUMP_HOST_DIR} -I${BUILDTOP}/tcpdump >>Makefile.local
echo CBOR_LIB=${BUILDTOP}/${HOST}/libcbor/src/libcbor.a      >>Makefile.local
echo CBOR_INCLUDE=-I${BUILDTOP}/include -Drestrict=__restrict__   >>Makefile.local
echo export ARCH LIBPCAP LIBPCAP_HOST LIBPCAPINC TCPDUMP NETDISSECTLIB NETDISSECTH >>Makefile.local
