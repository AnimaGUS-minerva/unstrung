#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include "oswlibs.h"
#include "rpl.h"
#include "counter_names.h"

/* split into seperate file, as designated initialization is a C99, not a C++ thing */

/* provide a count of discards */
const char *dag_network_packet_stat_names[PS_MAX+1]={
    [PS_SEQ_OLD]                = "sequence too old",
    [PS_PACKET_RECEIVED]        = "packets received",
    [PS_PACKET_PROCESSED]       = "packets processed",
    [PS_PACKETS_WATCHED]        = "packets received due to watch",
    [PS_LOWER_RANK_CONSIDERED]  = "packets with <dagrank",
    [PS_LOWER_RANK_REJECTED]    = "packets with <dagrank rejected",
    [PS_SUBOPTION_UNDERRUN]     = "packets where subopt was too short",
    [PS_SELF_PACKET_RECEIVED]   = "packets from self that were ignored",
    [PS_DAO_PACKET_RECEIVED]    = "DAO packets received",
    [PS_DIO_PACKET_RECEIVED]    = "DIO packets received",
    [PS_DAO_PACKET_IGNORED]     = "DAO packets ignored (non-local DODAG id)",
    [PS_DIO_PACKET_IGNORED]     = "DIO packets ignored (non-local DODAG id)",
    [PS_DAG_CREATED_FOR_WATCHING]="DAG created due to watch",
    [PS_SAME_PARENT_IGNORED]    = "packet with same parent ignored",
    [PS_SAME_SEQUENCE_IGNORED]  = "packet with same sequence ignored",
    [PS_DAOACK_PACKET_IGNORED]  = "DAOACK with unknown DAGID",
    [PS_DAOACK_NO_DAGID_IGNORED]= "DAOACK with missing DAGID",
    [PS_DAOACK_WRONG_PARENT]    = "DAOACK from incorrect parent",
    [PS_DAOACK_WRONG_SEQNO]     = "DAOACK with wrong SeqNo",
    [PS_RPL_UNKNOWN_CODE]       = "unknown RPL type code",
    [PS_DIS_PACKET_RECEIVED]    = "DIS packets received",
    [PS_DIS_PACKET_IGNORED]     = "DIS packets ignored",
    [PS_NEIGHBOUR_UNICAST_REACHABILITY] = "Neighbour Soliciation for Unreachability Detection",
    [PS_MAX]                    = "max reason"
};


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

