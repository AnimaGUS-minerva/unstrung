#!/bin/sh

SENDDAO=${SENDDAO-./senddao}

mkdir -p OUTPUTS
# SHOULD fail because no -O out.pcap

${SENDDAO} --fake -i wlan0 -v --dagid thisismydicedag2 --sequence 11 --instance 43 2>&1 | 
  tee OUTPUTS/senddao-test-03.raw | diff - senddao-test-03.out
