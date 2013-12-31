#!/bin/sh

tcpdump -n -r dioE-eth1.pcap -w dioE-eth1b.pcap src fe80::1000:ff:fe66:6602
tcpdump -n -r dioE-eth1b.pcap -w dioE-eth1c.pcap dst ff02::1a
tcpdump -n -r dioE-eth1c.pcap -w dioE-eth1d.pcap -c 1
