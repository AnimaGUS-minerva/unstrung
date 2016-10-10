#!/bin/sh

apt-get install dpkg-dev gcc-multilib g++-multilib pkg-config libssl1.0.0 libssl-dev
#cp /usr/lib/x86_64-linux-gnu/libcrypto* /tmp
apt-get install build-essential lib32stdc++6 libdbus-1-dev libusb-1.0-0-dev:i386 libusb-1.0-0-dev:amd64 lib32z1-dev flex bison libssl-dev:i386 libboost-dev libcmocka-dev libcmocka0:i386 libcmocka0:amd64 cmake
#mv /tmp/libcrypto* /usr/lib/x86_64-linux-gnu


