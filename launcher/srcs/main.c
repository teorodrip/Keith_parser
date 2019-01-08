/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   main.c                                                                   */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 14:21:03 by Mateo                                    */
/*   Updated: 2019/01/08 10:57:30 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"
#define EXTERN
#include "../includes/server.h"

void end_proper(client_t *cli, server_t *srv)
{
	client_t *tmp;
	while (cli)
		{
			tmp = cli->next;
			close(cli->client_fd);
			free(cli);
			cli = tmp;
		}
	close(srv->server_fd);
}

int main()
{
	PGconn *conn;
	server_t srv;
	client_t *cli;
	tickers_t tickers;
	uint32_t pos;
	/* unsigned char machines_running; */

	queue_g = NULL;
	conn = connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
	get_data(conn, SQL_ALL_REQ, &tickers);
	PQfinish(conn);
	init_server(&srv);
	cli = NULL;
	pos = 0;
	/* machines_running = 0; */
	while (pos < tickers.len || queue_g != NULL)// || machines_running)//c++ parsing
		{
			accept_client(&srv, &cli);
			read_clients(&cli);
			usleep(100000);
		}

	clean_tickers(&tickers);
	end_proper(cli, &srv);
}
