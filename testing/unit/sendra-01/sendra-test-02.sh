#!/bin/sh

SENDRA=${SENDRA-./sendra}

${SENDRA} --fake -v -d ../INPUTS/basic-dio.txt | tee OUTPUT/sendra-test-02.raw | diff - sendra-test-02.out
