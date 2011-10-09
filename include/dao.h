#ifndef _UNSTRUNG_DAO_H_
#define _UNSTRUNG_DAO_H_

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <string.h>
#include "rpl.h"
}

#include "rplmsg.h"
#include "node.h"

class rpl_dao : public rpl_msg {
public:
    rpl_dao(unsigned char *data, int dao_len);
    struct rpl_dao_target *rpltarget(void);
};


#endif /* NODE_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

