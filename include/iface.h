#ifndef _UNSTRUNG_IFACE_H_
#define _UNSTRUNG_IFACE_H_

extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <sys/time.h>
#include <netinet/icmp6.h>
#include <linux/if.h>             /* for IFNAMSIZ */
#include "oswlibs.h"
#include "rpl.h"
}

#define HWADDR_MAX 16
#include "prefix.h"
#include "event.h"
#include "debug.h"

enum network_interface_exceptions {
    TOOSHORT = 1,
};

class network_interface {

public:
    bool mark;
    bool                    rpl_grounded;

    int announce_network();
    network_interface();
    network_interface(int fd);
    network_interface(const char *if_name);

    // setup the object, open sockets, etc.
    bool setup(void);

    int errors(void) {
        return error_cnt;
    }

    void set_debug(class rpl_debug *deb) {
        debug = deb;
    }

    virtual int nisystem(const char *cmd);
    virtual void receive_packet(struct in6_addr ip6_src,
                                struct in6_addr ip6_dst,
                                time_t          now,
                                const u_char *bytes, const int len);

    void receive_dao(const u_char *dao_bytes, const int dao_len);
    void receive_dio(struct in6_addr from,
                     time_t          now,
                     const u_char *dio_bytes, const int dio_len);

    void send_dio(void);
    void send_dao(void);
    virtual void send_raw_icmp(unsigned char *icmp_body, unsigned int icmp_len);
    virtual bool faked(void);

    int  build_dio(unsigned char *buff, unsigned int buff_len, ip_subnet prefix);
    int  build_dao(unsigned char *buff, unsigned int buff_len, ip_subnet prefix);

    void set_if_name(const char *ifname);
    const char *get_if_name(void) { return if_name; };
    int        get_if_index(void);

    bool if_ifaddr(struct in6_addr ia) {
        return (memcmp(&ia, &if_addr, sizeof(ia))==0);
    };

    void set_rpl_dagid(const char *dagstr);
    void set_rpl_dagrank(const unsigned int dagrank) {
        rpl_dagrank = dagrank;
    };
    void set_rpl_sequence(const unsigned int sequence) {
        rpl_sequence = sequence;
    };
    void set_rpl_instanceid(const unsigned int instanceid) {
        rpl_instanceid = instanceid;
    };
    void set_rpl_prefixlifetime(const unsigned int lifetime) {
        rpl_lifetime = lifetime;
    };
    void set_rpl_version(const unsigned int version) {
        rpl_version = version;
    };
    void set_rpl_prefix(const ip_subnet prefix);
    void set_rpl_interval(const int msec);
    rpl_node    *my_dag_node(void);
    dag_network *my_dag_net(void);

    void set_rpl_mode(enum RPL_DIO_MOP m) {
        rpl_mode = m;
    };
    void set_rpl_nomulticast() {
        if(rpl_mode == RPL_DIO_NONSTORING ||
           rpl_mode == RPL_DIO_NONSTORING_MULTICAST) {
            rpl_mode = RPL_DIO_NONSTORING;
        } else if(rpl_mode == RPL_DIO_STORING ||
           rpl_mode == RPL_DIO_STORING_MULTICAST) {
            rpl_mode = RPL_DIO_STORING;
        }
    }

    void set_rpl_multicast() {
        if(rpl_mode == RPL_DIO_NONSTORING ||
           rpl_mode == RPL_DIO_NONSTORING_MULTICAST) {
            rpl_mode = RPL_DIO_NONSTORING_MULTICAST;
        } else if(rpl_mode == RPL_DIO_STORING ||
           rpl_mode == RPL_DIO_STORING_MULTICAST) {
            rpl_mode = RPL_DIO_STORING_MULTICAST;
        }
    }

    void update_multicast_time(void) {
        struct timeval tv;

        gettimeofday(&tv, NULL);

        last_multicast_sec = tv.tv_sec;
        last_multicast_usec = tv.tv_usec;
    };
    bool addprefix(prefix_node &prefix);

    /* eui string functions */
    char *eui48_str(char *str, int strlen);
    char *eui64_str(char *str, int strlen);

    /* find a dag network associated with the interface */
    dag_network       *find_or_make_dag_by_dagid(const char *name);
        
    static void scan_devices(rpl_debug *deb);
    static void main_loop(FILE *verbose, rpl_debug *debug);
    static network_interface *find_by_ifindex(int ifindex);
    static network_interface *find_by_name(const char *name);
    static int foreach_if(int (*func)(network_interface*, void*), void*arg);
    static void remove_marks(void);

    /* event lists */
    static event_map              things_to_do;

protected:
    static int    gather_linkinfo(const struct sockaddr_nl *who,
                                  struct nlmsghdr *n, void *arg);

    static int    adddel_linkinfo(const struct sockaddr_nl *who,
                                  struct nlmsghdr *n, void *arg);
    static int    adddel_ipinfo(const struct sockaddr_nl *who,
                                struct nlmsghdr *n, void *arg);

    /* debugging */
    rpl_debug              *debug;
    rpl_node               *node;
    dag_network            *dagnet;
    int                     if_index;      /* cached value for get_if_index()*/
    struct in6_addr         if_addr;

    /* maintain list of all interfaces */
    void add_to_list(void);

private:
    int packet_too_short(const char *thing, const int avail, const int needed);
    int                     nd_socket;
    int                     error_cnt;

    char                    if_name[IFNAMSIZ];
    int			if_prefix_len;

    uint8_t			if_hwaddr[HWADDR_MAX];
    int			if_hwaddr_len;

    int			if_maxmtu;

    /* list states */
    bool                    on_list;
    bool                    alive;

    /* RiPpLe statistics */
    enum RPL_DIO_MOP        rpl_mode;
    unsigned int            rpl_sequence;
    unsigned int            rpl_instanceid;
    unsigned int            rpl_dagrank;
    unsigned int            rpl_lifetime;
    unsigned int            rpl_version;
    unsigned char           rpl_dagid[16];
    unsigned int            rpl_dio_lifetime;
    ip_subnet               rpl_prefix;
    char                    rpl_prefix_str[SUBNETTOT_BUF];

    unsigned int            rpl_interval_msec;
        

    /* timers */
    time_t			last_multicast_sec;
    suseconds_t		last_multicast_usec;
        
    unsigned char          *control_msg_hdr;
    unsigned int            control_msg_hdrlen;

    /* read from our network socket and process result */
    void receive(time_t now);

    /* private helper functions */
    void setup_allrouters_membership(void);
    void check_allrouters_membership(void);

    /* space to format various messages */
    int append_suboption(unsigned char *buff,
                             unsigned int buff_len,
                             enum RPL_SUBOPT subopt_type,
                             unsigned char *subopt_data,
                             unsigned int subopt_len);
    int append_suboption(unsigned char *buff,
                             unsigned int buff_len,
                             enum RPL_SUBOPT subopt_type);
    int build_prefix_dioopt(ip_subnet prefix);
    int build_target_opt(ip_subnet prefix);

    unsigned char           optbuff[256];
    unsigned int            optlen;

    /* interface to netlink */
    void                    generate_eui64();
    unsigned char           eui48[6];
    unsigned char           eui64[8];
    prefix_map              ipv6_prefix_list;  /* always /128 networks */


    /* this is global to all the interfaces */
    class network_interface        *next;
    static class network_interface *all_if;
    static int                      if_count(void);

    static struct rtnl_handle      *netlink_handle;
    static bool                     open_netlink(void);

};

#define ND_OPT_RPL_PRIVATE_DAO 200
#define ND_OPT_RPL_PRIVATE_DIO 201

class iface_factory {
public:
    virtual network_interface *newnetwork_interface(const char *name);
};
extern class iface_factory *iface_maker;


#endif /* _UNSTRUNG_IFACE_H_ */

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: whitesmith
 * End:
 */

