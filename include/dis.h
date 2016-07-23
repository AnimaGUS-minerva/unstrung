#ifndef _UNSTRUNG_DIS_H_
#define _UNSTRUNG_DIS_H_

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

class rpl_dis : public rpl_msg {
public:
    rpl_dis(const unsigned char *data, int dis_len, u_int32_t *stats);
    struct rpl_dis_solicitedinfo *rplsolicitedinfo(void);
};


#endif /* _UNSTRUNG_DIS_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

