extern char *progname;

class sunshine_config {
	int nd_socket;

	void sunshing_config();
	int setup_sockets(void);
};

extern int announce_network(struct sunshine_config *sc);

