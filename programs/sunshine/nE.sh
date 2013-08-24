#!/bin/sh

/sbin/sunshine -K
sleep 1
/sbin/sunshine -v -i eth0 -i eth1 --verbose --verbose --dagid FUNFUNFUN  -W 10000
