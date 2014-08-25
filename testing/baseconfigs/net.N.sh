#
# network for N, between n1 and n3.
#  4E = ASCII for 'N'
#
if [ -n "$UML_n1_CTL" ]
then
    net_eth0="eth0=daemon,12:00:00:66:4E:01,unix,$UML_n1_CTL";
else
    net_eth0="eth0=mcast,12:00:00:66:4E:01,239.192.1.2,31200"
fi

if [ -n "$UML_n3_CTL" ]
then
    net_eth1="eth1=daemon,12:00:00:66:4E:02,unix,$UML_n3_CTL";
else
    net_eth1="eth1=mcast,12:00:00:66:4E:02,239.192.1.2,21303";
fi

net="$net_eth0 $net_eth1"




