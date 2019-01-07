/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   server.h                                                                 */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/07 10:47:53 by Mateo                                    */
/*   Updated: 2019/01/07 11:58:13 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT 8080
#define ADDR INADDR_ANY
#define MAX_CONNECTIONS 3
#define BUFF_SIZE 1024

typedef struct 	server_s
{
	int server_fd;
	unsigned int server_data_len;
	struct sockaddr_in server_data;
}								server_t;

#endif
