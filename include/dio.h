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

class rpl_dio {
public:
    rpl_dio(rpl_node &peer,
            const struct nd_rpl_dio *dio, int dio_len);
    struct nd_rpl_genoption *search_subopt(enum RPL_SUBOPT optnum,
                                           int *p_opt_len = NULL);
    struct rpl_dio_destprefix *destprefix(void);
    void   reset_options(void);
    
private:
    rpl_node                &mPeer;
    u_int8_t                mBytes[2048];
    int                     mLen;

    /*
     * this structure is a set of pseudo pointers,
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

