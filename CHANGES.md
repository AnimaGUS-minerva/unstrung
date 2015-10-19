Version 1.10
============

Version 1.10 was release on October 19, 2015.
- added listening on netlink socket to identify new interfaces
- new --ignore-pio to not configure a local IP based upon PIOs received in
the DIO.
- new --dao-if-filter and --dao-addr-filter permit addresses found on
interfaces listed to be added to DAOs sent.
- changes to event queue system: events can more easily be rescheduled
- the emitted DIO RPL prefix info option was created/parsed incorrectly.


Version 1.00
============

The first public release was version 1.00, on October 14, 2014,
but the exact release point become muddied as code was ported.
