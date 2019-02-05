// ************************************************************************** //
//                                                                            //
//                                                                            //
//   client.cpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/09 09:56:44 by Mateo                                    //
//   Updated: 2019/02/05 11:04:24 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"

// construct the client
client::client()
{
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = htons(ADDR);
}

// init the client ie connect to the server
void client::init()
{
	//create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			std::cerr << "Error: creating the socket\n";
			exit(EXIT_FAILURE);
		}
	// connect to the server
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			std::cerr << "Error: connecting the server\n";
			exit(EXIT_FAILURE);
		}
	// set the connection non blocking so if nothing has been sent the program can continue
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
		{
			std::cerr << "Error: setting fd flag\n";
			exit(EXIT_FAILURE);
		}
}

// get the header from the server to read all the tickers
char **client::init_bloom_tick(unsigned short *n_tickers, const char *buff, const int readed, unsigned int *buff_pos)
{
	char **bloom_tick;

	if (readed < META_INFO_LEN || buff[0] != 0x03)
		{
			std::cerr << "Error: recived bad code from server\n";
			exit(EXIT_FAILURE);
		}
	*n_tickers = *((unsigned short *)(buff + 1));
	bloom_tick = new char *[*n_tickers];
	(*buff_pos) += META_INFO_LEN;
	return(bloom_tick);
}

// get the tickers from the server with static buffer
char **client::get_tickers(unsigned short *n_tickers)
{
	uint8_t request[META_INFO_LEN + 1] = {0x03, 0x00, 0x00, 0x00};
	char buff[BUFF_SIZE];
	unsigned short len;
	unsigned int i, buff_pos;
	int readed;
	char **bloom_tick;
	unsigned char rem;
	bool finish = true;

	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0)
		{
			std::cerr << "Error: obtaining tickers from launcher\n";
			exit(EXIT_FAILURE);
		}
	bloom_tick = NULL;
	i = 0;
	buff_pos = 0;
	len = 0;
	rem = 0;
	while (finish)
		{
			// while there is something to read will read until get the total size of tickers
			if ((readed = read(sockfd, buff, BUFF_SIZE)) > 0)
				{
					buff_pos = 0;
					// if is the first loop init the tickers array
					if (!bloom_tick)
						bloom_tick = init_bloom_tick(n_tickers, buff, readed, &buff_pos);
					// read the readed buffer
					while (buff_pos < BUFF_SIZE && buff_pos < (unsigned int)readed)
						{
							// if there are remminent characters from previous read
							if (rem)
								{
									// fill the ticker that was incomplet
									memcpy(bloom_tick[i] + (len - rem), buff, rem);
									buff_pos += rem;
									i++;
									rem = 0;
								}
							// if there are no reminent
							else
								{
									// get the len of the next ticker
									len = buff[buff_pos++]; //this len includes null at end
									bloom_tick[i] = new char[len]; // allocate it
									if (len <= (readed - buff_pos)) // check if it fits in the buffer
										{
											// if fits just copy it
											memcpy(bloom_tick[i], buff + buff_pos, len);
											buff_pos += len;
											i++;
										}
									else
										{
											// if it does not fit copy until the end of buffer, and set reminnet for the next read
											memcpy(bloom_tick[i], buff + buff_pos, readed - buff_pos);
											rem = len - (readed - buff_pos);
											buff_pos += (readed - buff_pos);
										}
								}
						}
					// if we did all the tickers and the las ticker is not incomplet we have finnished
					if (i == *n_tickers && !rem)
						finish = false;
				}
		}
	return (bloom_tick);
}

// get the number of excels to watch
unsigned char client::get_watching_directories()
{
	uint8_t request[META_INFO_LEN + 1] = {0x07, 0x00, 0x00, 0x00};
	unsigned char vm_nb;

	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0)
		{
			std::cerr << "Error: obtaining directories to watch\n";
			exit(EXIT_FAILURE);
		}
	while (read(sockfd, &vm_nb, sizeof(unsigned char)) != sizeof(unsigned char));
	return (vm_nb);
}

// from here there are just same prototype functions to send signals see it in protocol.txt
void client::signal_shutdown(const unsigned char vm_nb)
{
	uint8_t request[META_INFO_LEN + 1] = {0x01, 0x01, 0x00, vm_nb};
	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0)
		{
			std::cerr << "Error: obtaining tickers from launcher\n";
			exit(EXIT_FAILURE);
		}
}

void client::signal_reboot(const unsigned char vm_nb)
{
	uint8_t request[META_INFO_LEN + 1] = {0x01, 0x01, 0x00, vm_nb};
	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0)
		{
			std::cerr << "Error: obtaining tickers from launcher\n";
			exit(EXIT_FAILURE);
		}
}

//to manage the queue (not needed in this version)
// void client::send_queue(const queue_t queue)
// {
// 	char buff[META_INFO_LEN + sizeof(queue_t)];

// 	buff[0] = 0x06;
// 	*((unsigned short *)(buff + 1)) = (unsigned short)sizeof(queue_t);
// 	*((queue_t *)(buff + META_INFO_LEN)) = queue;
// 	send(sockfd, buff, META_INFO_LEN + sizeof(queue_t), 0x0);
// }

void client::signal_parsing()
{
	char request[META_INFO_LEN + 1] = {SIG_PARS_RUN, 0x0, 0x0, 0x0};

	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0)
		{
			std::cerr << "Error: Sending SIG_PARS_RUN to server\n";
			exit(EXIT_FAILURE);
		}
}

void client::signal_end_parsing()
{
	char request[META_INFO_LEN + 1] = {SIG_PARS_NO_RUN, 0x0, 0x0, 0x0};

	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0)
		{
			std::cerr << "Error: Sending SIG_PARS_NO_RUN to server\n";
			exit(EXIT_FAILURE);
		}
}

unsigned char client::check_end()
{
	char request[META_INFO_LEN + 1] = {SIG_END, 0x0, 0x0, 0x0};
	unsigned char res;

	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0)
		{
			std::cerr << "Error: Sending SIG_END to server\n";
			exit(EXIT_FAILURE);
		}
	while (read(sockfd, &res, sizeof(unsigned char)) != sizeof(unsigned char));
	return (res);
}
