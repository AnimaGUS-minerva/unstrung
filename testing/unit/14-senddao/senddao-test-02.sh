#!/bin/sh

set -e
SENDDAO=${SENDDAO-./senddao}
PCAP02=../OUTPUTS/basic-dao.pcap

${SENDDAO} --fake -v -d ../INPUTS/basic-dao.txt -O ${PCAP02} -i wlan0 2>&1 | tee ../OUTPUTS/senddao-test-02.raw | diff - senddao-test-02.out

tcpdump -t -n -r ${PCAP02} -v -v | tee ../OUTPUTS/senddao-test-02-pcap.txt | diff - senddao-test-02-pcap.txt
