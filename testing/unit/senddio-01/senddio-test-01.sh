#!/bin/sh

SENDDIO=${SENDDIO-./senddio}

(
${SENDDIO} --fake -v -d ../INPUTS/just-comments.txt 
# with no file.
${SENDDIO} -v 
) | diff - senddio-test-01.out
