#!/bin/sh

set -e
BUILDTOP=$(cd ..; pwd)
(cd .. && curl http://www.ca.tcpdump.org/release/libpcap-1.6.2.tar.gz | tar xzf - )
(cd .. && curl http://www.ca.tcpdump.org/release/tcpdump-4.6.2.tar.gz | tar xzf - )
(cd .. && mkdir -p host/libpcap-1.6.2 && cd host/libpcap-1.6.2 && ../../libpcap-1.6.2/configure --prefix=${BUILDTOP} && make && make install)
(cd .. && mkdir -p host/tcpdump-4.6.2 && cd host/tcpdump-4.6.2 && ../../tcpdump-4.6.2/configure --prefix=${BUILDTOP} && make && make install)
(cd .. && mkdir -p x86/libpcap-1.6.2 && cd x86/libpcap-1.6.2 && CFLAGS=-m32 ../../libpcap-1.6.2/configure --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
(cd .. && ln -s x86/libpcap-1.6.2 libpcap && mkdir -p x86/tcpdump-4.6.2 && cd x86/tcpdump-4.6.2 && CFLAGS=-m32 ../../tcpdump-4.6.2/configure --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
echo LIBPCAP=${BUILDTOP}/x86/libpcap-1.6.2/libpcap.a >Makefile.local
echo LIBPCAP_HOST=${BUILDTOP}/host/libpcap-1.6.2/libpcap.a >>Makefile.local
echo LIBPCAPINC=-I${BUILDTOP}/include                >>Makefile.local
echo ARCH=i386  			             >>Makefile.local
echo TCPDUMP=${BUILDTOP}/host/tcpdump-4.6.2/tcpdump  >>Makefile.local
echo NETDISSECTLIB=${BUILDTOP}/host/tcpdump-4.6.2/libnetdissect.a >>Makefile.local
echo NETDISSECTH=-DNETDISSECT -I${BUILDTOP}/include -I${BUILDTOP}/host/tcpdump-4.6.2/ -I${BUILDTOP}/tcpdump-4.6.2 >>Makefile.local
echo export ARCH LIBPCAP LIBPCAP_HOST LIBPCAPINC TCPDUMP NETDISSECTLIB NETDISSECTH >>Makefile.local
