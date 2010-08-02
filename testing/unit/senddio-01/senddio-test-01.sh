#!/bin/sh

SENDDIO=${SENDDIO-./senddio}

# this test case just tests stderr, that the usage message comes out
# sanely, with no crashes when there are not the right arguments.

(
${SENDDIO} --fake -v -d ../INPUTS/just-comments.txt 
# with no file.
${SENDDIO} -v 
) 2>OUTPUT/senddio-test-01.err | diff - senddio-test-01.out
diff OUTPUT/senddio-test-01.err senddio-test-01.err
