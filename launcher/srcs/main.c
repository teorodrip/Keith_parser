/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   main.c                                                                   */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 14:21:03 by Mateo                                    */
/*   Updated: 2019/01/16 12:15:47 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#define EXTERN
#include "../includes/launcher.h"

void end_proper(client_t *cli, server_t *srv)
{
	client_t *tmp;

	for(int i = 0; i < VM_NB; i++)
		free(virtual_machines[i]);
	free(virtual_machines);

	while (cli)
		{
			tmp = cli->next;
			close(cli->client_fd);
			free(cli);
			cli = tmp;
		}
	close(srv->server_fd);
}

void init_virtual_machines()
{
	if (!(virtual_machines = (char **)malloc(sizeof(char *))))
		{
			dprintf(2, "Error: malloc init_vm\n");
			exit(EXIT_FAILURE);
		}
	virtual_machines[0] = strdup(VM_NAME_1);
	virtual_machines[1] = strdup(VM_NAME_2);
}

int main()
{
	PGconn *conn;
	server_t srv;
	client_t *cli_head;
	/* tickers_t tickers; */
	tickers_t tickers;
	/* unsigned char machines_running; */

	tickers = (tickers_t){0, 0, 0, NULL, NULL, NULL};
	conn = connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
	get_data(conn, SQL_ALL_REQ, &tickers);
	PQfinish(conn);
	init_server(&srv);
	cli_head = NULL;
	/* machines_running = 0; */
	while (tickers.pos < tickers.n_tuples || tickers.queue != NULL)// || machines_running)//c++ parsing
		{
			accept_client(&srv, &cli_head);
			read_clients(&cli_head, &tickers);
			usleep(100000);
		}

	/* clean_tickers(&tickers); */
	end_proper(cli_head, &srv);
}
