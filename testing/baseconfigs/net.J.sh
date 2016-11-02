#
# network for J, between n2 and n3.
#  4A = ASCII for 'J'
#
mac_eth0=12:00:00:66:4A:01
if [ -n "$UML_n2_CTL" ]
then
    net_eth0="eth0=daemon,$mac_eth0,unix,$UML_n2_CTL";
    kvm_eth0="-net nic,vlan=2,model=virtio,name=Jn2,macaddr=$mac_eth0 "
    kvm_eth0="$kvm_eth0 -net vde,vlan=2,sock=${VDE_DIR}/2";
else
    net_eth0="eth0=mcast,12:00:00:66:4A:01,239.192.0.1,21300"
fi

mac_eth1=12:00:00:66:4A:02
if [ -n "$UML_n3_CTL" ]
then
    net_eth1="eth1=daemon,$mac_eth1,unix,$UML_n3_CTL";
    kvm_eth1="-net nic,vlan=3,name=En3,macaddr=$mac_eth1,model=virtio"
    kvm_eth1="$kvm_eth1 -net vde,vlan=3,sock=${VDE_DIR}/3";
else
    net_eth1="eth1=mcast,12:00:00:66:4A:02,239.192.1.2,21303";
fi

mac_eth2=12:00:00:66:4A:03
if [ -d /tmp/vde.ctl ];
then
    kvm_eth2="-net nic,vlan=99,model=virtio,name=Jobiwan,macaddr=$mac_eth2 "
    kvm_eth2="$kvm_eth2 -net vde,vlan=99,sock=${VDE_DIR}/99";
fi


net="$net_eth0 $net_eth1"
kvmnet="$kvm_eth0 $kvm_eth1 $kvm_eth2"




