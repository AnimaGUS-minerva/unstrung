#!/bin/sh

SENDDIO=${SENDDIO-./senddio}
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


(
${SENDDIO} --fake -i wlan0 -v -I 42 -D thisismynicedag1 -p 2001:db8:0001::/48 -G --version 1 -S 10 -R 2 -s  -P 12
) | tee ../OUTPUTS/senddio-test-04.raw | diff -w -B - senddio-test-04.out

(
${SENDDIO} --fake -i wlan0 -v \
           --instance 42 \
           --dagid thisismynicedag1 --prefix 2001:db8:0001::/48 --prefixlifetime 12 \
           --grounded --storing --version 1 --sequence 10 --rank 2
) | tee ../OUTPUTS/senddio-test-04.raw | diff -w -B - senddio-test-04.out
