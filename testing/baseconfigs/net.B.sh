#
# network for B, between n1 and n2 (parallel to E)
#
if [ -n "$UML_n2_CTL" ]
then
    net_eth0="eth0=daemon,12:00:00:66:67:01,unix,$UML_n2_CTL";
else
    net_eth0="eth0=mcast,12:00:00:66:67:01,239.192.0.1,21300"
fi

if [ -n "$UML_n1_CTL" ]
then
    net_eth1="eth1=daemon,12:00:00:66:67:02,unix,$UML_n1_CTL";
else
    net_eth1="eth1=mcast,12:00:00:66:67:02,239.192.1.2,31200";
fi

net="$net_eth0 $net_eth1"




