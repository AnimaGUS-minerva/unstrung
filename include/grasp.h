#ifndef _GRASP_H_
#define _GRASP_H_

/* for random number generator for session-id */
/* see: https://tls.mbed.org/kb/how-to/add-a-random-generator */
/* kinda gross to suck in all of this just to get the sizes right */
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"


/* forward references */
class network_interface;
class rpl_debug;
struct cbor_item_t;

enum grasp_message {
  M_NOOP = 0,
  M_DISCOVERY = 1,
  M_RESPONSE = 2,
  M_REQ_NEG = 3,
  M_REQ_SYN = 4,
  M_NEGOTIATE = 5,
  M_END = 6,
  M_WAIT = 7,
  M_SYNCH = 8,
  M_FLOOD = 9};

enum grasp_options {
  O_DIVERT = 100,
  O_ACCEPT = 101,
  O_DECLINE = 102,
  O_IPv6_LOCATOR = 103,
  O_IPv4_LOCATOR = 104,
  O_FQDN_LOCATOR = 105,
  O_URI_LOCATOR = 106,
};

/*
 *    objective-flags = uint .bits objective-flag
 * objective-flag = &(
 *    F_DISC: 0 ; valid for discovery only
 *    F_NEG: 1 ; valid for discovery and negotiation
 *    F_SYNCH: 2) ; valid for discovery and synchronization
 */
enum objective_flags {
  F_DISC = 0,
  F_NEG  = 1,
  F_SYNCH= 2,
};

typedef u_int32_t grasp_session_id;

class grasp_client {
 public:
    grasp_client(rpl_debug *debug, network_interface *iface) {
        infd = -1;
        outfd= -1;
        deb = debug;
        this->iface = iface;
        init_query_client(iface);
        init_random();
        queries_outstanding = 0;
    };
    bool poll_setup(struct pollfd *fd1);
    bool open_connection(const char *serverip, unsigned int port);
    bool open_fake_connection(const char *outfile, const char *infile);
    bool send_cbor(cbor_item_t *query);
    cbor_item_t *read_cbor(void);
    int  server_fd(void) {
        return infd;
    };
    bool process_grasp_reply(time_t now);
    bool decode_grasp_reply(cbor_item_t *reply);
    grasp_session_id start_query_for_aro(unsigned char eui64[8]);
    virtual grasp_session_id generate_random_sessionid(bool init);

    void init_regress_random(void);

 private:
    int  infd;
    int  outfd;
    rpl_debug *deb;
    unsigned int  queries_outstanding;
    network_interface       *iface;
    bool                     entropy_init;
    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    void init_random(void);
    void init_query_client(network_interface *iface);
};

#endif /* _GRASP_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

