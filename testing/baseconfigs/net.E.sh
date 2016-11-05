#
# network for E, between n1 and n2.
#
mac_eth0=12:00:00:66:69:01
if [ -n "$UML_n2_CTL" ]
then
    net_eth0="eth0=daemon,$mac_eth0,unix,$UML_n2_CTL";
    kvm_eth0="-net nic,vlan=2,model=virtio,name=En2,macaddr=$mac_eth0 "
    kvm_eth0="$kvm_eth0 -net vde,vlan=2,sock=${VDE_DIR}/2";
else
    net_eth0="eth0=mcast,12:00:00:66:69:01,239.192.0.1,21300"
fi

mac_eth1=12:00:00:66:69:02
if [ -n "$UML_n1_CTL" ]
then
    net_eth1="eth1=daemon,$mac_eth1,unix,$UML_n1_CTL";
    kvm_eth1="-net nic,vlan=1,model=virtio,name=En1,macaddr=$mac_eth1 "
    kvm_eth1="$kvm_eth1 -net vde,vlan=1,sock=${VDE_DIR}/1";
else
    net_eth1="eth1=mcast,12:00:00:66:69:02,239.192.1.2,31200";
fi

mac_eth2=12:00:00:66:69:03
if [ -d /tmp/vde.ctl ];
then
    kvm_eth2="-net nic,vlan=99,model=virtio,name=Eobiwan,macaddr=$mac_eth2 "
    kvm_eth2="$kvm_eth2 -net vde,vlan=99,sock=${VDE_DIR}/99";
fi

net="$net_eth0 $net_eth1"
kvmnet="$kvm_eth0 $kvm_eth1 $kvm_eth2"






