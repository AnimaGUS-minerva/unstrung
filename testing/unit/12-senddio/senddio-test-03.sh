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

out=../OUTPUTS/senddio-test-03.pcap
rm -f $out

(
${SENDDIO} --pcapout $out --fake -i wlan0 -v \
    --dagid T1 --prefix 2001:db8:0001::/48 --prefixlifetime 12 \
    --instance 42 \
    --grounded --storing --version 1 --sequence 10 --rank 2
) | tee ../OUTPUTS/senddio-test-03.raw | diff -B -w - senddio-test-03.out

tcpdump -t -n -r $out -v -X | tee ../OUTPUTS/senddio-test-03-pcap.raw | diff -B -w - senddio-test-03-pcap.txt
