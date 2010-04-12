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
    { "kill",      0, 0, 'K'}, 
    { "verbose",   0, 0, 'v'}, 
    { name: 0 }, 
};

void usage()
{
    fprintf(stderr, "Usage: %s [-?] [-i ifname]\n", progname);
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
    while((c = getopt_long(argc, argv, "KDi:h?v", longopts, 0)) != EOF) {
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

        case 'v':
            verbose++;
            break;
        case 'i':
            ifname = optarg;
            break;
	}
    }

    if(!ifname) usage();

    network_interface *sc = new network_interface((const char *)ifname);
    sc->setup();
    sc->set_verbose(verbose, stderr);

    /* should check for already running instance before stomping PID file */

    if(bedaemon) {
        if(daemon(0, 1)!=0) {
            perror("daemon");
            exit(5);
        }
    }

    FILE *pidfile = fopen(pidfilename, "w");
    if(pidfile) {
        fprintf(pidfile, "%u", getpid());
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
