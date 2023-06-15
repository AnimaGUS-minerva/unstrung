#!/bin/sh

PECK=${PECK-../../../programs/peck/peck}

# this test case just tests stderr, that the usage message comes out
# sanely, with no crashes when there are not the right arguments.

(
${PECK} --help
${PECK} -V

# echo this one will fail as /boot/manufacturer.pem is not present, etc.
${PECK} --fake wlan0

# see SIGNING.md for instructions on where these came from
${PECK} --fake --mic mic.pem --manuca vendor_secp384r1.crt wlan0

) 2>&1 | tee OUTPUT/peck-test-01.raw | diff - peck-test-01.out
