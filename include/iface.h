class network_interface {

public:
	int announce_network();
	network_interface();
	network_interface(int fd);

	int errors(void) {
		return error_cnt;
	}
	void set_verbose(int flag) { verbose_flag = flag; }
	void set_verbose(int flag, FILE *out) {
		verbose_flag = flag;
		verbose_file = out;
	}
	int  verboseprint() { return verbose_flag; }

	virtual int send_packet(const u_char *bytes, const int len);
	virtual void receive_packet(struct in6_addr ip6_src,
				    struct in6_addr ip6_dst,
				    const u_char *bytes, const int len);
		

private:
	int packet_too_short(const char *thing, const int avail, const int needed);
	int nd_socket;
	int error_cnt;
	int verbose_flag;
	FILE *verbose_file;
#define VERBOSE(X) ((X)->verbose_flag && (X)->verbose_file!=NULL)
};


