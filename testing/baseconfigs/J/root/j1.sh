#!/bin/sh
if [ -x /sbin/sunshine ];
then
        mv /sbin/sunshine /sbin/sunshine.run
fi

/sbin/sunshine.run --dagid ripple -i eth0 -i eth1 --interval 100000 --verbose  --timelog
