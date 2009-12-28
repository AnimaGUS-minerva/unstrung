#!/bin/sh

TEST_PURPOSE=regress
TEST_PROB_REPORT=0
TEST_TYPE=pkttest

XHOST_LIST="A"
XNET_LIST="ground n1"

TESTNAME=senddio-02
AHOST=A
EXITONEMPTY=--exitonempty
PRIV_INPUT=../inputs/01-sunrise-sunset-ping.pcap

REF_N1_OUTPUT=n1-output.txt
REF_N1_FILTER="no-icmp-v6-mcast.sed"
REF_A_CONSOLE_OUTPUT=aconsole.txt
REF_CONSOLE_FIXUPS="script-only.sed"
A_RUN_SCRIPT=dio1.sh

PACKETRATE=100

