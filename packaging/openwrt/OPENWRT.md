Build instructions for OpenWRT
==============================

Download openwrt from git and do a trial build of it for your platform.
You will need various libaries and compilers built.
	git://git.openwrt.org/openwrt.git

The script packaging/openwrt/build-unstrung.sh contains a simple way to build the code
"in-tree" --- that is without creating a tar-ball or a git commit, etc.  It contains:

    make package/unstrung/prepare USE_SOURCE_DIR=/ssw/projects/pandora/unstrung V=s
    make package/unstrung/compile USE_SOURCE_DIR=/ssw/projects/pandora/unstrung V=s
    make package/unstrung/install USE_SOURCE_DIR=/ssw/projects/pandora/unstrung V=s

The script will need adjusting as it has the USE_SOURCE_DIR hard coded to the directory
where unstrung has been checked out.

To build, you will need to have libboost installed, which is 	
	CONFIG_boost-libs-all=y

As boost is just libraries (mostly in the form of C++ templates), there is no target 
size impact of just building it all.  In menuconfig, boost is located at:
	Libraries->boost

The result will be an ipkg file:
        ls -l bin/ramips/packages/base/unstrung_1.00-1_ramips_24kec.ipk


References
==========
	http://wiki.openwrt.org/doc/devel/packages
	http://wiki.openwrt.org/doc/howto/build
