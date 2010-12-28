#!/bin/sh

TEST_PURPOSE=regress
TEST_PROB_REPORT=0
TEST_TYPE=pkttest

XHOST_LIST="A"
XNET_LIST="ground n1"

TESTNAME=recvself-05
AHOST=A

N1_PLAY=../../unit/INPUTS/dio-02.pcap
REF_N1_OUTPUT=n1-output.txt
REF_N1_FILTER="no-icmp-v6-mcast.sed"
REF_A_CONSOLE_OUTPUT=econsole.txt
REF_CONSOLE_FIXUPS="script-only.sed "
#REF_CONSOLE_FIXUPS="${REF_CONSOLE_FIXUPS} remove_dummy.sed"
A_RUN_SCRIPT=asunshine.sh
A_FINAL_SCRIPT=sunfinal.sh

PACKETRATE=100

