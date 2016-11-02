#!/bin/sh
if [ -x /sbin/sunshine ];
then
	mv /sbin/sunshine /sbin/sunshine.run
fi

# for coredump
cd /tmp
ulimit -c unlimited

/sbin/sunshine.run -i 1 --dagid 2001:db8:0001::1 -i eth1 --rank 1 --prefix 2001:db8:0001::/48 --interval 30000 --verbose --timelog
