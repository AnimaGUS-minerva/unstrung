# Unstrung quick start guide


### Installation

In order to build Unstrung you have to have these libraries installed on your device
* boost 
* libnl3
* libpcap
* libusb

If you use any debian based distro(ex. raspbian) you can install them just by typing
```
sudo apt-get install libboost-all-dev libnl-3-dev libpcap-dev libusb-dev libusb-1.0-0-dev
```
*(it takes quite a long time on unpacking libboost1.55-dev,do not kill the process)*

Clone the source repository
```
git clone https://github.com/mcr/unstrung.git
cd unstrung/
make
```

## Quick start:

In order to create a DODAG you have to run sunshine
```
cd programms/sunshine
./sunshine -i lowpan0 -I 35 --prefix 2607:f018:800:201:c298:e588:4400:1/64 --dagid 0x11112222333344445555666677778888  -W 10000 --verbose -W 10000 --stderr -R 1 -m
```
*ATT:watch out the sequence of arguments,in different order a SIGSEGV0 may occur!*

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