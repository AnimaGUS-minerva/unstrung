#!/bin/sh

SENDDAO=${SENDDAO-../../../programs/senddao/senddao}
PCAP04=../OUTPUTS/senddao-test-04out.pcap

${SENDDAO} -v -v --fake --outpcap ${PCAP04} -i wlan0 -v --dagid T1 --sequence 1 --instance 2 --prefix 2001:db8::/38 --myid 2001:db8::abcd:1 --dest fe80::1200:00ff:fe64:6423 --target 2001:db8::abcd:2/128  2>&1 |
  tee ../OUTPUTS/senddao-test-04.raw | diff -B -w - senddao-test-04.out

tcpdump -t -n -r ${PCAP04} -v -v -v | tee ../OUTPUTS/senddao-test-04-pcap.txt | diff - senddao-test-04-pcap.txt

