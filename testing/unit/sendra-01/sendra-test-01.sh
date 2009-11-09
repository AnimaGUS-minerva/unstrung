#!/bin/sh

SENDRA=${SENDRA-./sendra}

${SENDRA} --fake -v -d ../INPUTS/just-comments.txt | diff - sendra-test-01.out
