// ************************************************************************** //
//                                                                            //
//                                                                            //
//   client.cpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/09 09:56:44 by Mateo                                    //
//   Updated: 2019/01/15 11:00:12 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"

client::client()
{
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = htons(ADDR);
}

void client::init()
{
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			std::cerr << "Error: creating the socket\n";
			exit(EXIT_FAILURE);
		}
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			std::cerr << "Error: connecting the server\n";
			exit(EXIT_FAILURE);
		}
}

char **client::get_tickers(short *n_tickers)
{
	uint8_t request[META_INFO_LEN + 1] = {0x03, 0x00, 0x00, 0x00};
	char buff[BUFF_SIZE];
	short len;
	int i, buff_pos;
	int readed;
	char **bloom_tick;
	unsigned char rem;

	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0 ||
			(readed = read(sockfd, buff, META_INFO_LEN)) != META_INFO_LEN ||
			buff[0] != 0x03)
		{
			std::cerr << "Error: obtaining tickers from launcher\n";
			exit(EXIT_FAILURE);
		}
	*n_tickers = *((short *)buff);
	bloom_tick = new char*[*n_tickers];
	i = 0;
	buff_pos = 0;
	len = 0;
	rem = 0;
	while ((readed = read(sockfd, buff, BUFF_SIZE)) > 0)
		{
			buff_pos = 0;
			while (buff_pos < BUFF_SIZE)
				{
					if (rem)
						{
							memcpy(bloom_tick[i], buff, rem);
							buff_pos += rem;
							rem = 0;
						}
					else
						{
							len = buff[buff_pos++]; //this len includes null at end
							bloom_tick[i] = new char[len];
							if (len < (readed - buff_pos))
								{
									memcpy(bloom_tick[i], buff + buff_pos, len);
									buff_pos += len;
									i++;
								}
							else
								{
									memcpy(bloom_tick[i], buff + buff_pos, readed - buff_pos);
									rem = len - (readed - buff_pos);
									buff_pos += (readed - buff_pos);
								}
						}
				}
		}
	return (bloom_tick);
}

unsigned char client::get_watching_directories()
{
	uint8_t request[META_INFO_LEN + 1] = {0x07, 0x00, 0x00, 0x00};
	unsigned char vm_nb;

	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0 ||
			read(sockfd, &vm_nb, sizeof(unsigned char)) != sizeof(unsigned char))
		{
			std::cerr << "Error: obtaining tickers from launcher\n";
			exit(EXIT_FAILURE);
		}
	return (vm_nb);
}


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
