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

#include "rplmsg.h"
#include "node.h"

class rpl_dio : public rpl_msg {
public:
    rpl_dio(rpl_node &peer,
            const struct nd_rpl_dio *dio, int dio_len);
    struct rpl_dio_destprefix *destprefix(void);

private:
    rpl_node                &mPeer;
};


#endif /* NODE_H */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

