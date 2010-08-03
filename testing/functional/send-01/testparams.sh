#!/bin/sh

TEST_PURPOSE=regress
TEST_PROB_REPORT=0
TEST_TYPE=pkttest

XHOST_LIST="A E"
XNET_LIST="ground n1"

TESTNAME=send-01
AHOST=A
EHOST=E

REF_N1_OUTPUT=n1-output.txt
REF_N1_FILTER="no-icmp-v6-mcast.sed"
REF_E_CONSOLE_OUTPUT=econsole.txt
REF_CONSOLE_FIXUPS="script-only.sed "
REF_CONSOLE_FIXUPS="${REF_CONSOLE_FIXUPS} remove_dummy.sed"
A_RUN_SCRIPT=asunshine.sh
E_RUN_SCRIPT=esunshine.sh
A_RUN2_SCRIPT=sleep5.sh
A_FINAL_SCRIPT=sunfinal.sh
E_FINAL_SCRIPT=sunfinal.sh

PACKETRATE=100

