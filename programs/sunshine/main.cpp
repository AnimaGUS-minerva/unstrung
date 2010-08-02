/*
 * Copyright (C) 2009 Michael Richardson <mcr@sandelman.ca>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sysexits.h>
#include <sys/types.h>
#include <signal.h>

#include "iface.h"

char *progname;
static struct option const longopts[] =
{
    { "help",      0, 0, '?'}, 
    { "interface", 0, 0, 'i'}, 
    { "daemon",    0, 0, 'D'}, 
    { "prefix",    1, NULL, 'p'},
    { "instance",  1, NULL, 'I'},
    { "interval",  1, NULL, 'W'},
    { "dagid",     1, NULL, 'G'},
    { "dagid",     1, NULL, 'G'},
    { "rank",      1, NULL, 'R'},
    { "kill",      0, 0, 'K'}, 
    { "verbose",   0, 0, 'v'}, 
    { name: 0 }, 
};

void usage()
{
    fprintf(stderr, "Usage: %s [-?] [-i ifname]\n", progname);
    fprintf(stderr,
            "\t [-p prefix] [--prefix prefix]   announce this IPv6 prefix in the destination option\n"
            "\t [-G dagid]  [--dagid dagid]     DAGid to use to announce, string or hexn\n"
            "\t [-R rank]   [--rank rank]       Initial rank to announce with\n"
            "\t [-I num]    [--instanceid num]  Instance ID (number)\n"
            "\t [-W msec]   [--interval msec]   Number of miliseconds between DIO\n"
        );
    exit(EX_USAGE);
}

const char *pidfilename="/var/run/sunshine.pid";
void killanydaemon(void)
{
    int pid;

    FILE *pidfile = fopen(pidfilename, "r");
    if(!pidfile) {
        fprintf(stderr, "could not found any running daemon\n");
        exit(1);
    }

    if(fscanf(pidfile, "%u", &pid) != 1) {
        fprintf(stderr, "could not decipher pid file contents\n");
        exit(2);
    }
    fclose(pidfile);

    printf("Killing process %u\n", pid);

    if(kill(pid, SIGTERM) != 0) {
        perror("kill");
        exit(3);
    }
   
    unlink(pidfilename);

    exit(0);
}

int main(int argc, char *argv[])
{
    int c;
    char *ifname = NULL;
    int verbose = 0;
    bool bedaemon = false;

    progname = argv[0];
    bool devices_scanned = false;

    network_interface *iface = NULL;


    while((c = getopt_long(argc, argv, "KDG:I:R:W:i:hp:?v", longopts, 0)) != EOF) {
	switch(c) {
	default:
	    fprintf(stderr, "Unknown option: %s\n", argv[optind-1]);
	    /* FALLTHROUGH */
	case 'h':
	case '?':
	    usage();
	    /* NORETURN */

        case 'D':
            bedaemon = true;
            break;

        case 'K':
            killanydaemon();
            exit(0);

        case 'p':
        {
            ip_subnet prefix;
            
            err_t e = ttosubnet(optarg, strlen(optarg),
                                AF_INET6, &prefix);

            if(!iface) usage();
            iface->set_rpl_prefix(prefix);
        }
        break;

        case 'I':
            if(!iface) usage();
            iface->set_rpl_instanceid(atoi(optarg));
            break; 

        case 'W':
            if(!iface) usage();
            iface->set_rpl_interval(atoi(optarg));
            break; 

        case 'R':
            if(!iface) usage();
            iface->set_rpl_dagrank(atoi(optarg));
            break; 

        case 'G':
            if(!iface) usage();
            iface->set_rpl_dagid(optarg);
            break;

        case 'v':
            verbose++;
            break;

        case 'i':
            if(!devices_scanned) {
                network_interface::scan_devices();
                devices_scanned = true;
            }
            iface = network_interface::find_by_name(optarg);
            break;
	}
    }

    if(!iface) usage();

    if(iface) iface->set_verbose(verbose, stderr);

    /* should check for already running instance before stomping PID file */

    if(bedaemon) {
        if(daemon(0, 1)!=0) {
            perror("daemon");
            exit(5);
        }
    }

    FILE *pidfile = fopen(pidfilename, "w");
    if(pidfile) {
        fprintf(pidfile, "%u\n", getpid());
        fclose(pidfile);
    }

    network_interface::main_loop(stderr);

    exit(0);
}


/*
 * Local Variables:
 * mode: c
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
