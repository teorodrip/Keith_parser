/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   main.c                                                                   */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 14:21:03 by Mateo                                    */
/*   Updated: 2019/02/05 12:31:33 by Mateo                                    */
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

int main(int argc, char **argv)
{
	server_t srv;
	client_t *cli_head;
	tickers_t tickers;
	uint64_t flags;
	int tmp_pos = 0;

	if (argc == 2)
		tmp_pos = atoi(argv[1]);

	flags = 0x0;
	tickers = (tickers_t){0, 0, tmp_pos, NULL, NULL, NULL};
	// if a header with the tickers is detected the tickers will be
	// loaded staticly in memory
#ifdef TICKERS_H
	char tickers_main[N_TUPLES][N_COLS][255] = TICKERS;
	unsigned char tick_len_main[N_TUPLES][N_COLS] = TICK_LEN;
	tickers.n_tuples = N_TUPLES;
	tickers.n_cols = N_COLS;
	tickers.tick_len = &tick_len_main;
	tickers.tickers = &tickers_main;
	// if not will be loaded from the data base
#else
	PGconn *conn;
	// connect to the data base and get the tickers
	conn = connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
	get_data(conn, SQL_ALL_REQ, &tickers);
	PQfinish(conn);
#endif
	// start the server
	init_server(&srv);
	cli_head = NULL;
	// loop until:
	// no tickers in list,
	// no tickers in queue (in this version there are no queue because is not needed, will implement that in the modular version)
	// no vm is running
	// the parse is also not running
	while (!(flags & F_END_2))
		{
			// if there are new clients accept them
			accept_client(&srv, &cli_head);
			// check for the clients querys
			read_clients(&cli_head, &tickers, &flags);
			// don't use all the computer
			usleep(100000);
		}
	end_proper(cli_head, &srv);
}
