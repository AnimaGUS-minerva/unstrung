#
# network for M, on n3 only
#  4D = ASCII for 'M'
#
if [ -n "$UML_n3_CTL" ]
then
    net_eth0="eth0=daemon,12:00:00:66:4D:01,unix,$UML_n3_CTL";
else
    net_eth0="eth0=mcast,12:00:00:66:4D:01,239.192.1.2,31200"
fi

net="$net_eth0 "




