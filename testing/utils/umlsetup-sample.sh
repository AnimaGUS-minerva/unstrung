#!/bin/sh

# This is the configuration file that helps setup for
# a kernel pool for UML compilation w/UNSTRUNG.
#
# Copy this file to the top of your UNSTRUNG source directory as
# umlsetup.sh, edit that copy, and populate the paths.


# space for everything:
# Just a shorthand for the following definitions.
# Can be eliminated if available space is fragmented.
UMLPREFIX=/uml

# set this to someplace with at least 100Mb free.
POOLSPACE=$UMLPREFIX/umlbuild

# Set this to original kernel source.
# It will not be modified.
# Could be native build:
#KERNPOOL=/usr/src/linux
#
# or something you downloaded (or 3.9 works)
KERNPOOL=/distros/linux-3.5
#KERNPOOL=/distros/linus.git

# set BASICROOT this to an unpacked copy of the root file system you
# want to use.
#
# a small-ish one is at:
#     http://junk.sandelman.ca/umlroot/
#
# umlroot-39.. is 57Mb, unpacks to around 280M.
#
# I did
#   mkdir -p $UMLPREFIX/basic-root
#   cd $UMLPREFIX/basic-root
#   nftp -o - http://junk.sandelman.ca/freeswan/uml/umlfreeroot-12.0.tar.gz | tar xzvf -
#  (or ncftp, or whatever your favorite program is)
#
# There is an advantage to having this on the same partition as
# $POOLSPACE, as hard links can be used, which speeds everything up, and
# makes it use many times less disk space.
#
BASICROOT=$UMLPREFIX/root

# the mini /usr/share has Canada zoneinfo and "en" locale only.
# the all one has everything from the original UML debian root.
# I run debian, so I can just use my native /usr/share!
SHAREDIR=/usr/share

# This describes the testing network that you want.
UNTI_HOSTS='A E J K N M'
UNTI_NETS="n1 n2 n3"

# set this to determine where the RAM/SWAP for the usermode linux goes.
export TMP=/uml/tmp
