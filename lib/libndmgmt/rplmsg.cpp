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
#include <string.h>

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <time.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/if.h>           /* for IFNAMSIZ */
#include "oswlibs.h"
#include "rpl.h"

}

#include "iface.h"
#include "dag.h"
#include "rplmsg.h"
#include "dio.h"
#include "dao.h"

/*
 * here are routines to crack open suboptions of DIO/DAG/etc. messages
 */
rpl_msg::rpl_msg(const unsigned char *subopts, int subopt_len)
{
    reset_options();

    if(subopt_len > sizeof(mBytes)) subopt_len = sizeof(mBytes);
    memcpy(mBytes, subopts, subopt_len);
    mLen = subopt_len;
}

void rpl_msg::reset_options(void)
{
    /* clear options */
    for(int i=0; i<256; i++) mOptions[i]=0;
}

/*
 * This routine looks for another option of a particular type.
 * If mOptions[optnum] is 0, then we start from the beginning.
 * If mOptions[optnum] is -1, then we found everything.
 * otherwise, we start scanning at that offset.
 *
 * If we find a different option, and mOptions[subopt] is 0, then
 * we update it to that location.
 */

struct nd_rpl_genoption *rpl_msg::search_subopt(enum RPL_SUBOPT optnum,
                                                int *p_opt_len)
{
    u_int8_t*opt_bytes = mBytes;
    int opt_left_len = mLen;
    int offset = 0;

    if(p_opt_len) *p_opt_len = 0;

    /* if we have already scanned to end of options... */
    if(mOptions[optnum] == -1) return NULL;

    /* else initialize to mOptions[optnum] */
    offset       += mOptions[optnum];
    opt_bytes    += offset;
    opt_left_len -= mOptions[optnum];

    while(opt_left_len > sizeof(struct rpl_dio_genoption)) {
        int skip_len = 0;

        /* make sure we do not do unaligned accesses */
        struct rpl_dio_genoption opt;
        memcpy((void*)&opt, (void*)opt_bytes,sizeof(struct rpl_dio_genoption));

        if(opt.rpl_dio_type == RPL_OPT_PAD0) {
            skip_len = 1;
        } else {
            skip_len = opt.rpl_dio_len + 2;
            if(opt.rpl_dio_len == 0) break;
        }

        /* if mOptions[dio_type] is 0, then advance it */
        if(mOptions[opt.rpl_dio_type]==0) {
            mOptions[opt.rpl_dio_type]=offset;
        }

        /* see if we found what we were looking for... ? */
        if(opt.rpl_dio_type == optnum) {
            mOptions[opt.rpl_dio_type]=offset+skip_len;
            if(p_opt_len) *p_opt_len = skip_len;
            return (struct nd_rpl_genoption *)opt_bytes;
        }

        opt_left_len -= skip_len;
        opt_bytes    += skip_len;
        offset       += skip_len;
    }
    return NULL;
}

struct rpl_dio_destprefix *rpl_dio::destprefix(void)
{
    int optlen = 0;
    struct rpl_dio_destprefix *dp = (struct rpl_dio_destprefix *)search_subopt(RPL_DIO_DESTPREFIX, &optlen);

    if(dp==NULL) return NULL;

    int prefixbytes = ((dp->rpl_dio_prefixlen+7) / 8)-1;
    if(prefixbytes > (optlen - sizeof(struct rpl_dio_destprefix))) {
        //(*mStats)[PS_SUBOPTION_UNDERRUN]++;
        return NULL;
    }

    return dp;
}


rpl_dio::rpl_dio(rpl_node &peer,
                 const struct nd_rpl_dio *dio, int dio_len) :
    mPeer(peer),
    rpl_msg((unsigned char *)(dio+1), dio_len-sizeof(*dio))
{
}


rpl_dao::rpl_dao(unsigned char *data, int dao_len) :
    rpl_msg(data, dao_len)
{
}



struct rpl_dao_target *rpl_dao::rpltarget(void)
{
    int optlen = 0;
    struct rpl_dao_target *rpltarget = (struct rpl_dao_target *)search_subopt(RPL_DAO_RPLTARGET, &optlen);

    if(rpltarget==NULL) return NULL;

    /* XXX validate the length */
    unsigned int prefixbytes = (rpltarget->rpl_dao_prefixlen+7)/8;
    if(optlen < (prefixbytes + sizeof(struct rpl_dao_target))) {
        return NULL;
    }

    return rpltarget;
}


/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */
