#!/bin/sh
if [ -x /sbin/sunshine ];
then
        mv /sbin/sunshine /sbin/sunshine.run
fi

# for coredump
cd /tmp
ulimit -c unlimited

/sbin/sunshine.run -I 1 -i eth0 -i eth1 --interval 100000 --verbose  --timelog
