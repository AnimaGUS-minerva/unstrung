#!/bin/sh

SENDDIO=${SENDDIO-./senddio}

${SENDDIO} --fake -v -S 10 -p : | tee OUTPUT/senddio-test-04.raw | diff - senddio-test-04.out
