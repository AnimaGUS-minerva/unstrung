Ripple (by the Grateful Dead)
======

if my words did glow
With the gold of sunshine
and my tunes were played on the harp unstrung
would you hear my voice
come through the music
would you hold it near
as it were your own

its a hand me down
the thoughts are broken
perhaps
they're better left unsung

ripple, in still water
where there is no pebble tossed
nor wind to blow.

reach out your hand,
if your cup is empty
if your cup is full,
may be be again.

=====

RPL, pronounced Ripple is an IETF protocol that provides connectivity among
nodes that can not all hear each other, but rather have to form a multihop
mesh-like network.

Unlike other mesh efforts, this all happens at layer-3, in IPv6
ICMP messages (originally, was going to be in Router Solicitation/ND messages).

The authors thinks that once RPL-style "mesh" routing is loose on
citizen controlled mobile devices that citizens will no longer be
held hostage to incumbent telcos that dominate the broadband "Internet".

This set of programs, plus a test bed/simulation system is a set of tools
that runs under the Linux kernel:
	- on netbooks,
	- laptops, and
	- on phones like the Google Android, Raspberry PI, OpenWRT, etc..

Programs include:
	sunshine	- run this on root/grounded server to announce
			routes and on any other well provisioned machines.

	glow		- run this on very underpowered machines that
                        just need enough to join the DAG [PLANNED]

	pebble		- insert DIO messages into the network, run from
			the command line [currently: senddao, senddio]

	blow		- cmd line utility to control sunshine and glow.
                        [PLANNED]

===== community

Github:
    http://github.com/mcr/unstrung.git/

Rudimentary web site:
    http://unstrung.sandelman.ca/

to subscribe to mailing list:
    https://lists.sandelman.ca/mailman/listinfo/unstrung-hackers


====== Testing environment

This is based upon the UML-Network Testing Infrastructure (UNTI... pronounced
like the british "aUNTIe")
UML is UserModeLinux -- a completely paravirtualized virtualization
       technnology from the turn of the century.

The UMLroot image that goes with it can not be created as stock wheezy
"debootstrap" image.  An image that works can be found at:
        http://junk.sandelman.ca/umlroot/umlroot-38.tgz

In addition, there are extensive unit tests, which are invokved by
"make unitcheck", and which run on travis-ci.org, see:
      https://travis-ci.org/mcr/unstrung/

Michael Richardson <mcr@sandelman.ca>,
Ottawa, Ontario, October 2009-2014.




