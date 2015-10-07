#ifndef _UNSTRUNG_RPLMSG_H_
#define _UNSTRUNG_RPLMSG_H_

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <string.h>
#include "rpl.h"
}

class rpl_msg {
public:
    rpl_msg(const unsigned char *subopts, int subopt_len, u_int32_t *stats);
    struct nd_rpl_genoption *search_subopt(enum RPL_SUBOPT optnum,
                                           int *p_opt_len = NULL);
    void   reset_options(void);

 protected:
    u_int32_t              *mStats;

private:
    u_int8_t                mBytes[2048];
    int                     mLen;

    /*
     * this structure is a set of pseudo pointers,
     * where we last found an an option of that particular type.
     */
    int16_t                  mOptions[256];
};


#endif /* RPLMSG_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

