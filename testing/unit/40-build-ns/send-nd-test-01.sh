#!/bin/sh

set -e
SENDND=${SENDND-../../../programs/sendnd/sendnd}
mkdir -p ../OUTPUTS

PCAP02=../OUTPUTS/basic-nd.pcap

echo "file ${SENDND}" >.gdbinit
echo "set args --help" >>.gdbinit
(${SENDND}; ${SENDND} --help) 2>&1 | tee ../OUTPUTS/send-nd-test-00.raw | diff -B -w - send-nd-test-00.out

ARGS="--fake -v -O ${PCAP02} -i wlan0 --solicit devid"
echo "set args ${ARGS}" >>.gdbinit
${SENDND} ${ARGS} 2>&1 | tee ../OUTPUTS/send-nd-test-01.raw | diff -B -w - send-nd-test-01.out

${TCPDUMP-tcpdump} -t -n -r ${PCAP02} -v -v | tee ../OUTPUTS/send-nd-test-01-pcap.txt | diff -B -w - send-nd-test-01-pcap.txt
