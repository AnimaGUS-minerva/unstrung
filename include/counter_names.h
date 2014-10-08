#ifndef _UNSTRUNG_COUNTER_NAMES_H_
#define _UNSTRUNG_COUNTER_NAMES_H_

/* split into seperate file to be C compatible */

enum packet_stats {
    PS_SEQ_OLD,
    PS_PACKET_RECEIVED,
    PS_PACKET_PROCESSED,
    PS_PACKETS_WATCHED,
    PS_LOWER_RANK_CONSIDERED,
    PS_LOWER_RANK_REJECTED,
    PS_SUBOPTION_UNDERRUN,
    PS_SELF_PACKET_RECEIVED,
    PS_DAO_PACKET_RECEIVED,
    PS_DIO_PACKET_RECEIVED,
    PS_DAO_PACKET_IGNORED,
    PS_DIO_PACKET_IGNORED,
    PS_DAG_CREATED_FOR_WATCHING,
    PS_SAME_PARENT_IGNORED,
    PS_SAME_SEQUENCE_IGNORED,
    PS_DAOACK_PACKET_IGNORED,
    PS_DAOACK_NO_DAGID_IGNORED,
    PS_DAOACK_WRONG_PARENT,
    PS_DAOACK_WRONG_SEQNO,
    PS_MAX,
};

extern const char *dag_network_packet_stat_names[PS_MAX+1];

#endif /* _UNSTRUNG_DAG_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

