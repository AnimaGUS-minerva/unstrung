#!/bin/sh

SENDDIO=${SENDDIO-./senddio}
set -e

(
${SENDDIO} --fake -i wlan0 -v -p 2001:db8:0001::/48 -G -I 42 --version 1 -S 10 -R 1 -s -D thisismynicedag1
) | tee OUTPUT/senddio-test-04.raw | diff - senddio-test-04.out

(
${SENDDIO} --fake -i wlan0 -v \
    --prefix 2001:db8:0001::/48 --prefixlifetime 10 \
   --instance 42 \
    --grounded --storing --version 1 --sequence 10 --rank 1 --dagid thisismynicedag1
) | tee OUTPUT/senddio-test-04.raw | diff - senddio-test-04.out
