PANDORA_SRCDIR?=$(shell cd ../../..; pwd)

include ${PANDORA_SRCDIR}/Makefile.inc
include ../Makefile.testing

export SENDDAO=${PANDORA_SRCDIR}/programs/senddao/senddao
PCAP01=../OUTPUTS/senddaoack-test-01out.pcap

checkprograms::
programs::
clean::
	@rm -f core.*
	@rm -rf OUTPUT

check::
	@rm -f core.*
	${SENDDAO} --fake --outpcap ${PCAP01} -i wlan0 -v --daoack --dagid thisismydicedag2 --sequence 11 --instance 43 2>&1 >../OUTPUTS/senddaoack-test-01.raw
	@diff -B -w ../OUTPUTS/senddaoack-test-01.raw senddaoack-test-01.txt
	@${TCPDUMP} -t -n -r ${PCAP01} -v -v -v >../OUTPUTS/senddaoack-test-01-pcap.txt
	@diff ../OUTPUTS/senddaoack-test-01-pcap.txt  senddaoack-test-01-pcap.txt
	@if [ -f core.* ]; then echo CORE FILES FOUND; exit 1; else exit 0; fi

update:
	cp ../OUTPUTS/senddaoack-test-01.raw         senddaoack-test-01.txt
	cp ../OUTPUTS/senddaoack-test-01-pcap.txt    senddaoack-test-01-pcap.txt
