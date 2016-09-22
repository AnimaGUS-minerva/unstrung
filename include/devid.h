#ifndef _UNSTRUNG_DEVID_H_
#define _UNSTRUNG_DEVID_H_

#include <map>

extern "C" {
#include <errno.h>
#include <signal.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <sys/time.h>
#include <netinet/icmp6.h>
#include <linux/if.h>             /* for IFNAMSIZ */
#include "oswlibs.h"
}

#define HWADDR_MAX 16
#include "unstrung.h"
#include "iface.h"

class device_identity {
public:
  int build_neighbour_solicit(network_interface *iface,
                             unsigned char *buff,
                             unsigned int buff_len);

  int build_neighbour_advert(network_interface *iface,
                             unsigned char *buff,
                             unsigned int buff_len);

protected:
  int foo;

private:
  int bar;
};

#endif /* _UNSTRUNG_DEVID_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

