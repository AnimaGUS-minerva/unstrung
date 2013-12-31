#!/bin/sh

/sbin/sunshine -K
sleep 2
/sbin/sunshine -v --dagid FUNFUNFUN -W 10000 -i eth0 -i eth1 2>&1 | tee -a /var/log/nE.log
