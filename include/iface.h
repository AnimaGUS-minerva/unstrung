class network_interface {

public:
	int announce_network();
	network_interface();
	network_interface(int fd);

	int errors(void) {
		return error_cnt;
	}

	virtual int send_packet(const u_char *bytes, const int len);
	virtual void receive_packet(const u_char *bytes, const int len);
		

private:
	int nd_socket;
	int error_cnt;
};


