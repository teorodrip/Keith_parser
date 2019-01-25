// ************************************************************************** //
//                                                                            //
//                                                                            //
//   client.hpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/25 09:33:32 by Mateo                                    //
//   Updated: 2019/01/25 11:05:09 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../../launcher/includes/logger.h"
#include "../../launcher/includes/protocol.h"

#define PORT 8080
#define ADDR INADDR_ANY

class client
{
private:
  struct sockaddr_in serv_addr;
  int sockfd;
  // char **init_bloom_tick(unsigned short *n_tickers, const char *buff, const int readed, unsigned int *buff_pos);

public:
  client();
  bool connect_server();
	bool send_data(const uint16_t length, const char *data);
	ssize_t get_data_blocking(const size_t buff_size, char *buff);
  // char **get_tickers(unsigned short *n_tickers);
  // unsigned char get_watching_directories();
  // void signal_shutdown(const unsigned char vm_nb);
  // void signal_reboot(const unsigned char vm_nb);
  // void send_queue(const queue_t queue);
	// void signal_parsing();
	// void signal_end_parsing();
	// unsigned char check_end();
};

#endif
