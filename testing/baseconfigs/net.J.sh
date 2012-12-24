#
# network for J, between n2 and n3. 
#  4A = ASCII for 'J'
#
if [ -n "$UML_n2_CTL" ]
then
    net_eth0="eth0=daemon,12:00:00:66:4A:01,unix,$UML_n2_CTL";
else
    net_eth0="eth0=mcast,12:00:00:66:4A:01,239.192.0.1,21300"
fi

if [ -n "$UML_n3_CTL" ]
then
    net_eth1="eth1=daemon,12:00:00:66:4A:02,unix,$UML_n3_CTL";
else
    net_eth1="eth1=mcast,12:00:00:66:4A:02,239.192.1.2,21303";
fi

net="$net_eth0 $net_eth1"




