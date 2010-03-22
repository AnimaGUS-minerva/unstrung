: ==== start ====
sleep 2
ip -6 addr ls
: /sbin/sunshine -i eth0 -i eth1
tcpdump -t -i eth1 -n &
echo true

