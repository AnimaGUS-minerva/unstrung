#ifndef _FAKE_GRASP_H_
#define _FAKE_GRASP_H_

#include "grasp.h"

class fake_grasp_client: public grasp_client {
 public:
  fake_grasp_client(rpl_debug *debug, network_interface *iface) : grasp_client(debug, iface) {
    non_random_session_id = 1804289382;
  };

  virtual grasp_session_id generate_random_sessionid(bool init) {
    return ++non_random_session_id;
  };

 private:
  unsigned int non_random_session_id;

};

#endif /* _FAKE_GRASP_H */
