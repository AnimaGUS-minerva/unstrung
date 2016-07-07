#!/bin/sh

PECK=${PECK-../../../programs/peck/peck}

# this test case just tests stderr, that the usage message comes out
# sanely, with no crashes when there are not the right arguments.

(
${PECK} --help
# with no file.
${PECK} -V
) 2>&1 | tee OUTPUT/peck-test-01.raw | diff - peck-test-01.out
