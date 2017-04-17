#!/bin/sh

BUILDTOP=`pwd`/stuff

mkdir -p $BUILDTOP
cd $BUILDTOP
[ -d mbedtls ] || git clone -b mcr_add_otherName https://github.com/mcr/mbedtls.git
(cd mbedtls && cmake -DCMAKE_INSTALL_PREFIX=$BUILDTOP . && make && make install )

touch Makefile.local
if ! grep MBEDTLS Makefile.local
then
    echo MBEDTLSH=-I${BUILDTOP}/include          >>Makefile.local
    echo MBEDTLSLIB=${BUILDTOP}/lib              >>Makefile.local
fi



