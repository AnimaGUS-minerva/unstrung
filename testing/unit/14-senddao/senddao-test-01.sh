#!/bin/sh

SENDDAO=${SENDDAO-../../../programs/senddao/senddao}

# this test case just tests stderr, that the usage message comes out
# sanely, with no crashes when there are not the right arguments.

mkdir -p OUTPUTS

(
${SENDDAO} --fake -v -d ../INPUTS/just-comments.txt 
# with no file.
${SENDDAO} -v 
) 2> OUTPUTS/senddao-test-01.err | tee OUTPUTS/senddao-test-01.raw | diff - senddao-test-01.out
diff OUTPUTS/senddao-test-01.err senddao-test-01.err
