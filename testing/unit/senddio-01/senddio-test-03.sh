#!/bin/sh

SENDDIO=${SENDDIO-./senddio}

${SENDDIO} --fake -v -p 2001:db8:0001::/48 | tee OUTPUT/senddio-test-03.raw | diff - senddio-test-03.out
