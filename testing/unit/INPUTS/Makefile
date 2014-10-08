all: dio-A-ripple1.pcap dio-E-eth1d.pcap
# first packet, from node A
dio-A-ripple1.pcap: dio-E-eth1.pcap
	tcpdump -c 1 -n -r dio-E-eth1.pcap -w dio-A-ripple1.pcap

# second packet, from node E.
dio-E-eth1d.pcap: dio-E-eth1.pcap
	tcpdump -c 1 -n -w dio-E-eth1d.pcap -r dio-E-eth1.pcap ip6 src fe80::1000:ff:fe66:6602