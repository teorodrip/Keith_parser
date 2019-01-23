/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   main.c                                                                   */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 14:21:03 by Mateo                                    */
/*   Updated: 2019/01/23 15:45:20 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#define EXTERN
#include "../includes/launcher.h"

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
	client_t *cli_head;
	tickers_t tickers;
	uint64_t flags;

	flags = 0x0;
	tickers = (tickers_t){0, 0, 0, NULL, NULL, NULL};
	conn = connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
	get_data(conn, SQL_ALL_REQ, &tickers);
	PQfinish(conn);
	init_server(&srv);
	cli_head = NULL;
	while (!(flags & F_END_2))
		{
			accept_client(&srv, &cli_head);
			read_clients(&cli_head, &tickers, &flags);
			usleep(100000);
		}
	end_proper(cli_head, &srv);
}
