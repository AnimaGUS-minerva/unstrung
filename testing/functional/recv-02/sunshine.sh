: ==== start ====
sleep 2
ip -6 addr ls
: /sbin/sunshine -i eth0 -i eth1
tcpdump -i eth1 -n &
: ==== end ====

