#!/bin/sh

SENDDIO=${SENDDIO-../../../programs/senddio/senddio}
set -e

#Usage: senddio [--verbose]
#               [--prefix prefix]     -p      2001:db8:0001::/48
#               [-d datafile]
#               [--prefixlifetime]    -P       12
#               [--fake]              -T      YES
#               [--iface net]         -i
#               [--version #]         -V        1
#               [--grounded]          -G      YES
#               [--storing]           -s      YES
#               [--non-storing]       -N
#               [--multicast]         -m
#               [--no-multicast]      -M
#               [--sequence #]        -S       10
#               [--instance #]        -I       42
#               [--rank #]            -R        2
#               [--dagid hexstring]   -D        thisismynicedag1

mkdir -p ../OUTPUTS
out=../OUTPUTS/senddio-test-03.pcap
rm -f $out

echo "file ${SENDDIO}" >.gdbinit
ARGS="--pcapout $out --fake -i wlan0 -v --instanceid 42 --dagid 2001:db8:661e::1 --prefix 2001:db8:0001::/48 --prefixlifetime 12 --grounded --storing --version 1 --sequence 10 --rank 2"
echo "set args ${ARGS}"  >>.gdbinit

( eval ${SENDDIO} ${ARGS} ) | tee ../OUTPUTS/senddio-test-03.raw | diff -B -w - senddio-test-03.out

${TCPDUMP-tcpdump} -t -n -r $out -v -X | tee ../OUTPUTS/senddio-test-03-pcap.raw | diff -B -w - senddio-test-03-pcap.txt
