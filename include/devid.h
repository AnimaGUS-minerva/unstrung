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

#include "mbedtls.h"

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

    /**
     * @brief load a device identity from a certificate
     * This function initializes a device_identity structure from attributes found
     * in a certificate.
     *
     * @param const char *ca_file,
     * @param const char *certfile
     */
    int load_identity_from_cert(const char *ca_file, const char *certfile);
    int extract_eui64_from_cert(unsigned char *eui64,
                                char *eui64buf, unsigned int eui64buf_len);

protected:
    bool                    eui64set;
    unsigned char           eui48[6];
    unsigned char           eui64[8];

private:
    mbedtls_x509_crt *cert;
};

#endif /* _UNSTRUNG_DEVID_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

