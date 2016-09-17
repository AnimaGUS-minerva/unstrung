#!/bin/sh

set -e
SENDND=${SENDND-../../../programs/sendnd/sendnd}
mkdir -p ../OUTPUTS

PCAP02=../OUTPUTS/basic-nd.pcap

${SENDND} --fake -v -O ${PCAP02} -i wlan0 2>&1 | tee ../OUTPUTS/send-nd-test-01.raw | diff -B -w - send-nd-test-01.out

${TCPDUMP-tcpdump} -t -n -r ${PCAP02} -v -v | tee ../OUTPUTS/send-nd-test-01-pcap.txt | diff -B -w - send-nd-test-01-pcap.txt
