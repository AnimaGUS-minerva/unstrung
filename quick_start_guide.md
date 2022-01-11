# Unstrung quick start guide


### Installation

In order to build Unstrung you have to have these libraries installed on your device
* boost
* libnl3
* libpcap
* libusb

If you use any debian based distro(ex. raspbian) you can install them just by typing
```
sudo apt-get install libboost-all-dev libnl-3-dev libpcap-dev \
                     libusb-dev libusb-1.0-0-dev libcmocka-dev cmake
```

The script install-apt-deps.sh can be used to get the most up-to-date set of packages.
On x86 it will attempt to install gcc-multilib, and install packages for both 32-bit and 64-bit compilations.

*(it takes quite a long time on unpacking libboost1.55-dev,do not kill the process)*

Clone the source repository
```
git clone https://github.com/mcr/unstrung.git
cd unstrung/
make
```

## Quick start:

In order to create a DODAG you have to run sunshine.  You'll want to have it
announce a prefix.   If you have a spare /64 which is routed to you, use it.
You can use a ULA as well.  Shown below is the documentation prefix.
```
cd programms/sunshine
./sunshine -i lowpan0 -I 35 \
           --prefix 2000:db8:1234:abcd::/64 \
           --dagid  2000:db8:1234::1 -W 10000 --verbose --stderr -R 1 -m
```
*ATT: there are reports that a different sequence of arguments may cause a SEGV. If that occurs, please open an issue.*

| Argument | explanation |
| ------ | ------ |
| -\-instanceid or \-I | the ID of your dodag instance.Just put any number |
| -\-prefix or \-p | the network prefix that your root will advertise.More information about it here:http://unique-local-ipv6.com/ |
| -\-dagid or \-G | the unique id of your DODAG.Put the ipv6 of your root in hex format |
| -\-rank or \-R | the rank of your node on the DODAG |
| -\-interval or \-W | The period between sending DIO's |
| -\-nomulticast or \-m | Disable multicast in DIO's |
| -\-verbose | Turn on logging |
| -\-stder | Log to stderr |