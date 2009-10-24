extern char *progname;

class neighbour_discovery {

public:
	int announce_network();
	neighbour_discovery();
	neighbour_discovery(int fd);

	int errors(void) {
		return error_cnt;
	}
		

private:
	int nd_socket;
	int error_cnt;
};


