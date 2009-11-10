#!/bin/sh

SENDDIO=${SENDDIO-./senddio}

${SENDDIO} --fake -v -d ../INPUTS/just-comments.txt | diff - senddio-test-01.out
