#!/usr/bin/expect --

#
# $Id: Xhost-test.tcl,v 1.19 2005/10/20 21:11:45 mcr Exp $
#

if {! [info exists env(UNSTRUNG_SRCDIR)]} {
    puts stderr "Please point \$UNSTRUNG_SRCDIR properly"
    exit 24
}

source $env(UNSTRUNG_SRCDIR)/testing/utils/GetOpts.tcl
source $env(UNSTRUNG_SRCDIR)/testing/utils/netjig.tcl

proc usage {} {
    puts stderr "Usage: Xhost-test "
    puts stderr "\t-D                 start up the UML nic so that there is DNS"
    puts stderr "\t-H host=path,host=path start up additional UMLs as specified"
    puts stderr "\t-n <netjigprog>    path to netjig program"
    puts stderr "\t-N <netjigprog>    extra stuff to send to netjig program"
    puts stderr "\t-a                 if netjig should enable --arpreply"
    puts stderr "\t-e <file>          pcap file to play on east network"
    puts stderr "\t-w <file>          pcap file to play on west network"
    puts stderr "\t-E <file>          record east network to file"
    puts stderr "\t-W <file>          record west network to file"
    puts stderr "\t-p <file>          pcap file to play on public network"
    puts stderr "\t-P <file>          record public network to file"
    puts stderr "\t-c <file>          file to send console output to"
    puts stderr "\n"
    puts stderr "The following environment variables are also consulted:\n"
    puts stderr "XHOST_LIST\tcontains a whitespace list of hosts which should be managed"
    puts stderr "\nFor each host, the following variables are examined:"
    puts stderr "\${HOST}_INIT_SCRIPT\tthe script to initialize the host with"
    puts stderr "\${HOST}_RUN_SCRIPT\tthe script to run the host with"
    puts stderr "\${HOST}_FINAL_SCRIPT\tthe script to run the host with"
    puts stderr "\${HOST}_START\tthe program to invoke the UML"
    puts stderr "REF_\${HOST}_CONSOLE_OUTPUT\twhere to redirect the console output to"
    puts stderr "PACKETRATE\tthe rate at which packets will be replayed"
    puts stderr "{NORTH,SOUTH,EAST,WEST}_PLAY denotes a pcap file to play on that network"
    puts stderr "{NORTH,SOUTH,EAST,WEST}_REC  denotes a pcap file to reocrd into from that network"
    exit 22
}

set umlid(someplay) 0
set do_dns           0

set timeout 100
log_user 0
if {[info exists env(HOSTTESTDEBUG)]} {
    if {$env(HOSTTESTDEBUG) == "hosttest"} {
	log_user 1
    }
}

netjigdebug "Program invoked with $argv"
set arpreply ""
set umlid(extra_hosts) ""

foreach net $managednets {
    process_net $net
}

while { [ set err [ getopt $argv "D:H:n:N:ae:E:w:W:p:P:" opt optarg]] } {
    if { $err < 0 } then {
	puts stderr "Xhost-test.tcl: $opt and $optarg" 
	usage
    } else {
	#puts stderr "Opt $opt arg: $optarg"

	switch -exact $opt {
	    D {
		process_extra_host "nic=$optarg"
	    }
	    H {
		process_extra_host $optarg
	    }
	    n {
		set netjig_prog $optarg
	    }
	    N {
		set netjig_extra $optarg
	    }
	    a {
		set arpreply "--arpreply"
	    }
	    e {
		set umlid(neteast,play) $optarg
		set umlid(neteast,setplay) 1
		set umlid(someplay) 1
	    }
	    w {
		set umlid(netwest,play) $optarg
		set umlid(netwest,setplay) 1
		set umlid(someplay) 1
	    }
	    p {
		set umlid(netpublic,play) $optarg
		set umlid(netpublic,setplay) 1
		set umlid(someplay) 1
	    }
	    E {
		set umlid(neteast,record) $optarg
		set umlid(neteast,setrecord) 1
	    }
	    W {
		set umlid(netwest,record) $optarg
		set umlid(netwest,setrecord) 1
	    }
	    P {
		set umlid(netpublic,record) $optarg
		set umlid(netpublic,setrecord) 1
	    }
	}
    }
}

if {! [info exists env(XHOST_LIST)]} {
    puts stderr "You must specify at least one host to manage in \$XHOST_LIST"
    exit 23
}

foreach net $managednets {
    calc_net $net
}

set managed_hosts [split $env(XHOST_LIST) ", "]

foreach host $managed_hosts {
    process_host $host
}

set argv [ lrange $argv $optind end ]

if {! [file executable $netjig_prog]} {
    puts "UML startup must be provided - did you run \"make checkprograms\"?"
    exit
}

netjigdebug "Starting up the netjig for $netjig_prog"
set netjig1 [netjigstart]


netjigsetup $netjig1

foreach net $managednets {
    newswitch $netjig1 $net
}

if {[info exists netjig_extra]} {
    playnjscript $netjig1 $netjig_extra
}

trace variable expect_out(buffer) w log_by_tracing

# start up auxiliary hosts first
foreach host $umlid(extra_hosts) {
    startuml $host
}
foreach host $umlid(extra_hosts) {
    loginuml $host
}
foreach host $umlid(extra_hosts) {
    initdns  $host
}

# now setup regular hosts

foreach host $managed_hosts {
    startuml $host
}

foreach host $managed_hosts {
    loginuml $host
}

# XXX two of the blank lines comes out here.
foreach host $managed_hosts {
    inituml $host
}

foreach net $managednets {
    if {[info exists umlid(net$net,record)] } {
	netjigdebug "Will record network '$net' to $umlid(net$net,record)"
	record $netjig1 $net $umlid(net$net,record)
    }
}

foreach net $managednets {
    if {[info exists umlid(net$net,play)] } {
	netjigdebug "Will play pcap file $umlid(net$net,play) to network '$net'"
	setupplay $netjig1 $net $umlid(net$net,play) ""
    }
}

# let things settle.
after 500

# see if we should wait
wait_user

# do the "run" scripts now.
foreach host $managed_hosts {
    runuml $host
}

# XXX the other blank line comes out during waitplay.
if { $umlid(someplay) == 0 } {
    netjigdebug "WARNING: There are NO PACKET input sources, not waiting for data injection"
} else {
    waitplay $netjig1
}

# run any additional scripts/passes until there are no passes left
set pass 2
set scriptcount 1
while {$scriptcount > 0} {
    set scriptcount 0
    netjigdebug "Attempting script pass $pass for $managed_hosts"
    foreach host $managed_hosts {
	set scriptcount [expr [runXuml $host $pass] + $scriptcount]
    }
    incr pass

    netjigdebug "Asking netjig for any output"
    expect -i $netjig1 -gl "*"
}
    
netjigdebug "Okay, done. Shutting down everything"

after 500
foreach host $managed_hosts {
    send -i $umlid($host,spawnid) "\r"
}

foreach host $managed_hosts {
    expect {
	-i $umlid($host,spawnid) -exact "# " {}
	timeout { 
	    puts "Can not find prompt prior to final script for $host (timeout)"
	    exit;
	}
	eof { 
	    puts "Can not find prompt prior to final script for $host (EOF)"
	    exit;
	}
    }
}

foreach host $managed_hosts {
    netjigdebug "Shutting down $host"
    killuml $host
}
sleep 5

foreach host $umlid(extra_hosts) {
    netjigdebug "Shutting down extra host: $host"
    shutdown $host
}

log_user 1
expect -i $netjig1 -gl "*"
send -i $netjig1 "quit\n"
set timeout 60
expect {
	-i $netjig1
	timeout { puts "expected EOF but got timeout in Xhost-test.tcl" }
	eof
}

