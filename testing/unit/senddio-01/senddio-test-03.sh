#!/bin/sh

SENDDIO=${SENDDIO-./senddio}

${SENDDIO} --fake -v -p : | tee OUTPUT/senddio-test-03.raw | diff - senddio-test-03.out
