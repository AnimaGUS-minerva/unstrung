#
# $Id: net.east.sh,v 1.6 2004/02/03 20:14:01 mcr Exp $
#
if [ -n "$UML_n2_CTL" ]
then
    net_eth0="eth0=daemon,12:00:00:66:66:01,unix,$UML_n2_CTL";
else
    net_eth0="eth0=mcast,12:00:00:66:66:01,239.192.0.1,21300"
fi

if [ -n "$UML_n1_CTL" ]
then
    net_eth1="eth1=daemon,12:00:00:66:66:02,unix,$UML_n1_CTL";
else
    net_eth1="eth1=mcast,12:00:00:66:66:02,239.192.1.2,31200";
fi

net="$net_eth0 $net_eth1"




