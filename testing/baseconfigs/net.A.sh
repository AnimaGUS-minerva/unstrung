#
# $Id: net.east.sh,v 1.6 2004/02/03 20:14:01 mcr Exp $
#
mac_eth0=12:00:00:dc:bc:ff
if [ -n "$UML_ground_CTL" ]
then
    net_eth0="eth0=daemon,$mac_eth0,unix,$UML_ground_CTL";
    kvm_eth0="-net nic,vlan=0,model=virtio,name=Aground,macaddr=$mac_eth0 "
    kvm_eth0="$kvm_eth0 -net vde,vlan=0,sock=${VDE_DIR}/0"
else
    mcast_eth0=239.192.0.1,21200
    net_eth0="eth0=mcast,$mac_eth0,$mcast_eth0"
    kvm_eth0="-net nic,vlan=0,name=Aground,macaddr=$mac_eth0,model=virtio -net socket,vlan=0,mcast=239.192.0.1:21200"
fi

mac_eth1=12:00:00:64:64:23
if [ -n "$UML_n1_CTL" ]
then
    net_eth1="eth1=daemon,$mac_eth1,unix,$UML_n1_CTL";
    kvm_eth1="-net nic,vlan=1,model=virtio,name=An1,macaddr=$mac_eth1 "
    kvm_eth1="$kvm_eth1 -net vde,vlan=1,sock=${VDE_DIR}/1"
else
    net_eth1="eth1=mcast,$mac_eth1,239.192.1.2,31200";
    kvm_eth1="-net nic,vlan=1,name=An1,macaddr=$mac_eth1,model=virtio -net socket,vlan=1,mcast=239.192.1.2:31200"
fi

mac_eth2=12:00:00:64:64:24
net_eth2=""
kvm_eth2=""
if [ -r /var/run/uml-utilities/uml_switch.ctl ];
then
    net_eth2="eth2=daemon,$mac_eth2,unix,/var/run/uml-utilities/uml_switch.ctl";
fi
if [ -d /tmp/vde.ctl ];
then
    kvm_eth2="-net nic,vlan=99,model=virtio,name=Aobiwan,macaddr=$mac_eth2 "
    kvm_eth2="$kvm_eth2 -net vde,vlan=99,sock=${VDE_DIR}/99";
fi

net_slip="ssl0=port:21201"
kvm_slip="-serial tcp::21201,server,nowait"

net="$net_eth0 $net_eth1 $net_eth2 $net_slip"
kvmnet="$kvm_eth0 $kvm_eth1 $kvm_eth2 $kvm_slip"




