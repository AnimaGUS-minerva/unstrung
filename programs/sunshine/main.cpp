/*
 * Copyright (C) 2009-2013 Michael Richardson <mcr@sandelman.ca>
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
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include "iface.h"
#include "dag.h"
#include "unstrung.h"
#include "grasp.h"
#include "devid.h"

#define OPTION_SYSLOG 0x01
#define OPTION_STDERR 0x02
#define OPTION_SLEEP  0x03

char *progname;
static struct option const longopts[] =
{
    { "help",      0, 0, '?'},
    { "interface", 1, 0, 'i'},
    { "daemon",    0, 0, 'D'},
    { "prefix",    1, NULL, 'p'},
    { "registrar", 1, NULL, 'r'},
    { "ignore-pio",0, NULL, 'P'},
    { "dao-if-filter",  1, NULL, 'A'},
    { "dao-addr-filter",1, NULL, 'a'},
    { "ldevid",    1, NULL, 'l'},
    { "iid",       1, NULL, 'd'},
    { "instance",  1, NULL, 'I'},
    { "instanceid",1, NULL, 'I'},
    { "syslog",    0, NULL,  OPTION_SYSLOG},
    { "stderr",    0, NULL,  OPTION_STDERR},
    { "sleep",     1, NULL,  OPTION_SLEEP},
    { "interval",  1, NULL, 'W'},
    { "dagid",     1, NULL, 'G'},
    { "rank",      1, NULL, 'R'},
    { "kill",      0, 0, 'K'},
    { "verbose",   0, 0, 'v'},
    { "timelog",   0, 0, 't'},
    { "nomulticast",   0, 0, 'm'},
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
            "\t [-m]        [--nomulticast]     Disable multicast in DIOs\n"
            "\t [--verbose] [--timelog]         Turn on logging (with --time logged)\n"
            "\t [--syslog]  [--stderr]          Log to syslog and/or stderr\n"
            "\t [--registrar hostname:port]     set address of GRASP responder on registrar\n"
            "\t [--ldevid filename]             load certificate with ACP Node Name to configure IID\n"
            "\t [--iid ipv6]                    setup the lower bits of the IPv6, the IID\n"
            "\t [--ignore-pio]                  Ignore PIOs found in DIO\n"
            "\t [--dao-if-filter]     List of interfaces (glob permitted) to take DAO addresses from\n"
            "\t [--dao-addr-filter]   List of prefixes/len to take DAO addresses from\n"
            "\t [--sleep=secs]                  sleep secs before trying to talk to network\n"
        );
    exit(EX_USAGE);
}

bool check_dag(unsigned char opt, dag_network *dag)
{
    if(!dag) {
	fprintf(stderr, "while processing '%c'; --instanceID must preceed DODAG parameters\n",
                opt);
	usage();
    }
    return true;
}

const char *piddirname ="/run/acp";
const char *pidfilename="/run/acp/sunshine.pid";
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

void write_pid_file()
{
    if(mkdir(piddirname, 0775) == -1 && errno != EEXIST) {
        fprintf(stderr, "Can not create PID directory %s: %s\n",
                piddirname, strerror(errno));
        exit(10);
    }

    FILE *pidfile = fopen(pidfilename, "w");
    int   mypid = getpid();
    if(pidfile) {
        fprintf(pidfile, "%u\n", mypid);
        fclose(pidfile);
    } else {
        fprintf(stderr, "Can not write pid=%u to %s: %s\n",
                mypid, pidfilename, strerror(errno));
    }
}

int main(int argc, char *argv[])
{
    int c;
    char *ifname = NULL;
    bool verbose = false;
    bool bedaemon = false;
    bool grounded = false;
    int loaded = 0;
    instanceID_t instanceID = 0;
    unsigned int grasp_portnum = 3000;
    char *grasp_registrar = NULL;
    device_identity di;
    err_t e1 = NULL;

    progname = argv[0];
    bool devices_scanned = false;

    network_interface *iface = NULL;
    rpl_debug *deb = new rpl_debug(false, stderr);
    deb->set_progname(progname);

    dag_network::init_stats();
    dag_network *dag = NULL;

    tzset();
    time_t now = time(NULL);
    char *today = ctime(&now);
    deb->info("PANDORA unstrung version %u.%u (%s) starting at %s",
              PANDORA_VERSION_MAJOR, PANDORA_VERSION_MINOR,
              BUILDNUMBER, today);

    /* process one argument, if it's --sleep then do that first */
    c = getopt_long(argc, argv, "KDG:I:R:W:i:hl:p:?v", longopts, 0);
    if(c == OPTION_SLEEP) {
        unsigned int doze=atoi(optarg);
        write_pid_file();
        if(doze > 0) sleep(doze);
    }
    /* reset argument processor */
    optind = 1;

    if(network_interface::scan_devices(deb, true) != true) {
        exit(10);
    }
    devices_scanned=true;

    while((c = getopt_long(argc, argv, "KDG:I:R:W:i:hp:r:m?v", longopts, 0)) != EOF) {
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

        case 't':
            deb->want_time_log = true;
            break;

        case OPTION_STDERR:
            deb->log_file   = true;
            break;

        case OPTION_SYSLOG:
            deb->log_syslog = true;
            break;

        case OPTION_SLEEP:
#if 0
            {
                unsigned int doze=atoi(optarg);
                if(doze > 0) sleep(doze);
            }
#endif
            break;

        case 'r':
            {
                char *portstr = strrchr(optarg, ':');
                if(portstr) {
                    *portstr = '\0';
                    portstr++;
                    grasp_portnum = atoi(portstr);
                }
                grasp_registrar = strdup(optarg);
            }
            break;

        case 'l':
#ifdef HAVE_MBEDTLS
            loaded = di.load_identity_from_cert(NULL, optarg);
            if(loaded != 0) {
                fprintf(stderr, "could not load ldevid certificate from %s\n", optarg);
                usage();
            }
            if(!di.parse_rfc8994cert()) {
                fprintf(stderr, "could not parse ldevid certificate from %s\n", optarg);
                usage();
            }
            check_dag(c, dag);
            dag->set_acp_identity(&di);
            {
                char sbuf[SUBNETTOT_BUF];
                subnettot(&di.sn, 0, sbuf, sizeof(sbuf));
                deb->info("set up IID from certificate %s with subnet %s\n",
                          optarg, sbuf);
            }
#else
            fprintf(stderr, "sunshine not built with MBEDTLS, can not parse identity from ldevid certificate\n");
            usage();
#endif /*  HAVE_MBEDTLS */
            break;

        case 'd':
            check_dag(c, dag);
            e1 = ttoaddr(optarg, 0, AF_INET6, &di.sn.addr);
            if(e1 != NULL) {
                deb->info("failed to parse %s as IPv6 subnet: %s\n",
                          optarg, e1);
                usage();
                break;
            }
            di.sn.maskbits = 128;
            dag->set_acp_identity(&di);
            {
                char sbuf[SUBNETTOT_BUF];
                subnettot(&di.sn, 0, sbuf, sizeof(sbuf));
                deb->info("set up IID from %s giving subnet %s\n",
                          optarg, sbuf);
            }
            break;

        case 'p':
        {
            ip_subnet prefix;

            err_t e = ttosubnet(optarg, strlen(optarg),
                                AF_INET6, &prefix);

	    if(e) {
		fprintf(stderr, "invalid prefix string: %s\n", optarg);
		usage();
	    }
	    check_dag(c, dag);
            dag->set_prefix(prefix);
            dag->set_grounded(true);
            dag->set_dagrank(1);
            grounded = true;
        }
        break;

        case 'P':
            check_dag(c, dag);
            dag->set_ignore_pio(true);
            break;

        case 'A':
            check_dag(c, dag);
            dag->set_interface_wildcard(optarg);
            break;

        case 'a':
            check_dag(c, dag);
            dag->set_interface_filter(optarg);
            break;

        case 'I':
            instanceID = atoi(optarg);
	    dag = new dag_network(instanceID, deb);
	    dag->set_active();
            dag->set_interval(5000);
            break;

        case 'W':
	    check_dag(c, dag);
            dag->set_interval(atoi(optarg));
            break;

        case 'R':
	    check_dag(c, dag);
	    {
		int rank = atoi(optarg);
		dag->set_dagrank(rank);
		//fprintf(stderr, "setting rank for %s to %u\n", dag->get_dagName(),
		//    rank, dag->get_dagRank());
		dag->set_sequence(1);
	    }
            break;

        case 'G':
            check_dag(c, dag);
            dag->set_dagid(optarg);
            break;

        case 'v':
            verbose=true;
            deb->set_verbose(stderr);
            deb->set_debug_flags(0xffffffff);  /* improvise for now */
            break;

        case 'i':
            iface = network_interface::find_by_name(optarg);
            if(!iface) {
                deb->log("Can not find interface %s\n", optarg);
                continue;
            } else {
		deb->verbose("Setting up interface[%d] %s\n",
			     iface->get_if_index(),
			     iface->get_if_name());

                iface->set_debug(deb);
                iface->setup();

#ifdef GRASP_CLIENT
                if(grasp_registrar) {
                    grasp_client *gc = new grasp_client(deb, iface);
                    if(gc->open_connection(grasp_registrar, grasp_portnum) != true) {
                        exit(21);
                    }
                }
#endif
            }

            /* build a DIS, send it. */
            if(dag && !grounded) {
                iface->send_dis(dag);
            }
            //printf("hatype for: %u\n", iface->get_hatype());
            break;

        case 'm':
            check_dag(c, dag);
            dag->set_nomulticast();
            break;
	}
    }

    if (!iface) {
        dag->add_all_interfaces();
    } else {
        dag->addselfprefix(iface);
    }

    if(!iface) {
	deb->info("running on all interfaces\n");
	network_interface::setup_all_if();
    }

    if(dag==NULL) {
	fprintf(stderr, "must set at least one DAG to announce\n");
	usage();
    }

    /* if no identity has been loaded, then make one up for self,
     * if we are the root
     */
    if(loaded == 0 && grounded) {
        dag->add_all_interfaces();
    }

    /* should check for already running instance before stomping PID file */

    if(bedaemon) {
        if(daemon(0, 1)!=0) {
            perror("daemon");
            exit(5);
        }
    }

    write_pid_file();

    dag->set_debug(deb);
    dag->schedule_dio(IMMEDIATELY);

    network_interface::main_loop(stderr, deb);

    exit(0);
}


/*
 * Local Variables:
 * mode: c
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
