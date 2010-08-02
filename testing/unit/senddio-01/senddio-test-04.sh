#!/bin/sh

SENDDIO=${SENDDIO-./senddio}
set -e

(
${SENDDIO} --fake -i wlan0 -v -p 2001:db8:0001::/48 -S 10 -I 42 -R 1 -D thisismynicedag1
) | tee OUTPUT/senddio-test-04.raw | diff - senddio-test-04.out

(
${SENDDIO} --fake -i wlan0 -v --prefix 2001:db8:0001::/48 --instance 42 \
    --sequence 10 --rank 1 --dagid thisismynicedag1
) | tee OUTPUT/senddio-test-04.raw | diff - senddio-test-04.out
