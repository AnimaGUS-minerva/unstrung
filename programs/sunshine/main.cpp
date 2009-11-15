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
#include <getopt.h>
#include <sysexits.h>

#include "iface.h"

char *progname;
static struct option const longopts[] =
{
    { "help",      0, 0, '?'}, 
    { "interface", 0, 0, 'i'}, 
    { "verbose",   0, 0, 'v'}, 
    { name: 0 }, 
};

void usage()
{
    fprintf(stderr, "Usage: %s [-?] [-i ifname]\n", progname);
    exit(EX_USAGE);
}

int main(int argc, char *argv[])
{
    int c;
    char *ifname = NULL;
    int verbose = 0;

    progname = argv[0];
    while((c = getopt_long(argc, argv, "i:h?v", longopts, 0)) != EOF) {
	switch(c) {
	default:
	    fprintf(stderr, "Unknown option: %s\n", argv[optind-1]);
	    /* FALLTHROUGH */
	case 'h':
	case '?':
	    usage();
	    /* NORETURN */

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
