#!/bin/sh

if [ `uname -m` = "amd64" ] || [ `uname -m` = "i386" ]
then
    sudo apt-get install gcc-multilib g++-multilibi lib32stdc++6 lib32z1-dev
fi
sudo apt-get install dpkg-dev pkg-config libssl1.0.0 libssl-dev
#cp /usr/lib/x86_64-linux-gnu/libcrypto* /tmp

sudo apt-get install build-essential libdbus-1-dev libusb-1.0-0-dev flex bison libboost-dev libcmocka-dev libcmocka0 \
     cmake libnl-3-200 libnl-genl-3-200 libnl-3-dev libnl-genl-3-dev libusb-1.0-0 \
     libpcap-dev



