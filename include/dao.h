#ifndef _UNSTRUNG_DIO_H_
#define _UNSTRUNG_DIO_H_

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <string.h>
#include "rpl.h"
}

#include "node.h"

class rpl_dao {
public:
    rpl_dao(rpl_node &peer,
            const struct nd_rpl_dao *dao, int dao_len);
    struct nd_rpl_genoption *search_subopt(enum RPL_SUBOPT optnum,
                                           int *p_opt_len = NULL);
    void   reset_options(void);
    
private:
    rpl_node                &mPeer;
    u_int8_t                mBytes[2048];
    int                     mLen;

    /*
     * this structure is a set of virtual pointers,
     * where we last found an an option of that particular type.
     */
    int16_t                  mOptions[256]; 
};


#endif /* NODE_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

