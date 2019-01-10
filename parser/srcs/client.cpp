// ************************************************************************** //
//                                                                            //
//                                                                            //
//   client.cpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/09 09:56:44 by Mateo                                    //
//   Updated: 2019/01/10 16:56:13 by Mateo                                    //
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

size_t client::get_number_tickers()
{
	uint8_t request[META_INFO_LEN + 1] = {0x03, 0x00, 0x00, 0x00};
	size_t data_size;
	int readed;

	if (send(sockfd, request, META_INFO_LEN + 1, 0) < 0 ||
			(readed = read(sockfd, &data_size, sizeof(size_t))) != sizeof(size_t))
		{
			std::cerr << "Error: obtaining tickers from launcher\n";
			exit(EXIT_FAILURE);
		}
	return (data_size);
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
