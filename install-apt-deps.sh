#!/bin/sh

apt-get install dpkg-dev gcc-multilib g++-multilib pkg-config libssl1.0.0 libssl-dev
#cp /usr/lib/x86_64-linux-gnu/libcrypto* /tmp

apt-get install build-essential lib32stdc++6 libdbus-1-dev:amd64 libusb-1.0-0-dev:amd64 lib32z1-dev flex bison libboost-dev libcmocka-dev:amd64 libcmocka0:amd64 cmake libnl-3-200:amd64 libnl-genl-3-200:amd64 libnl-3-dev:amd64 libnl-genl-3-dev:amd64 libusb-1.0-0

#apt-get install libcmocka0:i386 libnl-3-200:i386 libnl-genl-3-200:i386
#apt-get install libdbus-1-dev:i386 libusb-1.0-0-dev:i386 libcmocka-dev:i386 libcmocka0:i386 libnl-3-dev:i386 libnl-genl-3-dev:i386

#mv /tmp/libcrypto* /usr/lib/x86_64-linux-gnu


