/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   server.h                                                                 */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/07 10:47:53 by Mateo                                    */
/*   Updated: 2019/01/08 10:29:04 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 8080
#define ADDR INADDR_ANY
#define MAX_CONNECTIONS 3
#define BUFF_SIZE 1024
#define META_INFO_LEN 3 // 1 byte for code 2 bytes for length

typedef struct queue_s
{
	uint32_t start; //inclusive
	uint32_t end; //exclusive
	struct queue_s *next;
} queue_t;

typedef struct 	server_s
{
	int server_fd;
	unsigned int server_data_len;
	struct sockaddr_in server_data;
}								server_t;

typedef struct 	client_s
{
	int client_fd;
	char *name;
	struct client_s *next;
}								client_t;

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN queue_t *queue_g;

void accept_client(const server_t *srv, client_t **cli);
void init_server(server_t *srv);
void read_clients(client_t **head);
void decode_data(const char *buff, const ssize_t readed);

#endif
