#!/bin/sh

set -e
SENDND=${SENDND-../../../programs/sendnd/sendnd}
mkdir -p ../OUTPUTS

PCAP42=../OUTPUTS/basic-na.pcap
echo "file ${SENDND}" >.gdbinit

ARGS="--fake -v -O ${PCAP42} -i wlan0 --advert fe80::1200:ff:fe66:4d01"
echo "set args ${ARGS}" >>.gdbinit
${SENDND} ${ARGS} 2>&1 | tee ../OUTPUTS/send-nd-test-42.raw | diff -B -w - send-nd-test-42.out

${TCPDUMP-tcpdump} -t -n -r ${PCAP42} -v -v | tee ../OUTPUTS/send-nd-test-42-pcap.txt | diff -B -w - send-nd-test-42-pcap.txt
