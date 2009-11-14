#!/bin/sh

SENDDIO=${SENDDIO-./senddio}

(
${SENDDIO} --fake -v -d ../INPUTS/just-comments.txt 
# with no file.
${SENDDIO} -v 
) 2>OUTPUT/senddio-test-01.err | diff - senddio-test-01.out
diff OUTPUT/senddio-test-01.err senddio-test-01.err
