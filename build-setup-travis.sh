#!/bin/sh

set -e
(cd .. && curl http://www.ca.tcpdump.org/release/libpcap-1.6.2.tar.gz | tar xzf - )
(cd .. && curl http://www.ca.tcpdump.org/release/tcpdump-4.6.2.tar.gz | tar xzf - )
(cd .. && mkdir -p host/libpcap-1.6.2 && cd host/libpcap-1.6.2 && ../../libpcap-1.6.2/configure --prefix=`pwd` && make && make install)
(cd .. && mkdir -p host/tcpdump-4.6.2 && cd host/tcpdump-4.6.2 && ../../tcpdump-4.6.2/configure --prefix=`pwd` && make && make install)
(cd .. && mkdir -p x86/libpcap-1.6.2 && cd x86/libpcap-1.6.2 && CFLAGS=-m32 ../../libpcap-1.6.2/configure --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
(cd .. && ln -s x86/libpcap-1.6.2 libpcap && mkdir -p x86/tcpdump-4.6.2 && cd x86/tcpdump-4.6.2 && CFLAGS=-m32 ../../tcpdump-4.6.2/configure --target=i686-pc-linux-gnu && make LDFLAGS=-m32 CFLAGS=-m32)
echo LIBPCAP=$(pwd)/../x86/libpcap-1.6.2/libpcap.a >Makefile.local
echo LIBPCAPINC=-I$(pwd)/../include                >>Makefile.local
echo TCPDUMP=$(pwd)/../host/tcpdump-4.6.2/tcpdump  >>Makefile.local
echo NETDISSECTLIB=$(pwd)/../host/tcpdump-4.6.2/libnetdissect.a >>Makefile.local
echo NETDISSECTH=-DNETDISSECT -I$(pwd)/../include -I$(pwd)/../host/tcpdump-4.6.2/ -I$(pwd)/../tcpdump-4.6.2 >>Makefile.local
