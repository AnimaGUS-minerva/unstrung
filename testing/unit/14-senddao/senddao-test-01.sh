#!/bin/sh

SENDDAO=${SENDDAO-./senddao}

# this test case just tests stderr, that the usage message comes out
# sanely, with no crashes when there are not the right arguments.

(
${SENDDAO} --fake -v -d ../INPUTS/just-comments.txt 
# with no file.
${SENDDAO} -v 
) 2>../OUTPUTS/senddao-test-01.err | diff - senddao-test-01.out
diff ../OUTPUTS/senddao-test-01.err senddao-test-01.err
