#
# network for M, on n3 only
#  4D = ASCII for 'M'
#
mac_eth0=12:00:00:66:4D:01
if [ -n "$UML_n3_CTL" ]
then
    net_eth0="eth0=daemon,$mac_eth0,unix,$UML_n3_CTL";
    kvm_eth0="-net nic,vlan=3,name=Mn3,macaddr=$mac_eth0,model=virtio"
    kvm_eth0="$kvm_eth0 -net vde,vlan=3,sock=${VDE_DIR}/3";
else
    net_eth0="eth0=mcast,$mac_eth0,239.192.1.2,31200"
fi

mac_eth2=12:00:00:66:4D:03
if [ -d /tmp/vde.ctl ];
then
    kvm_eth2="-net nic,vlan=99,model=virtio,name=Mobiwan,macaddr=$mac_eth2 "
    kvm_eth2="$kvm_eth2 -net vde,vlan=99,sock=${VDE_DIR}/99";
fi


net="$net_eth0 "
kvmnet="$kvm_eth0 $kvm_eth2"



