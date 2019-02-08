/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   main.c                                                                   */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 14:21:03 by Mateo                                    */
/*   Updated: 2019/02/08 11:19:47 by Mateo                                    */
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

static char *mystrcat(char *dest, char *src)
{
	while (*dest)
		dest++;
	while ((*dest++ = *src++));
	return (--dest);
}

static void intersect_tickers(tickers_t *tickers)
{
	char request[N_TUPLES * MAX_TICK_LEN * 4];
	char *tmp = request;
	PGresult *res;
	PGconn *conn;

	request[0] = '\0';
	tmp = mystrcat(tmp, "SELECT * FROM unnest(ARRAY[");
	for (size_t i = 0; i < tickers->n_tuples; i++)
		{
			tmp = mystrcat(tmp, "\'");
			tmp = mystrcat(tmp, (*(tickers->tickers))[i][0]);
			tmp = mystrcat(tmp, "\', ");
		}
	tmp -= 2;
	*tmp = '\0';
	tmp = strcat(tmp, "], ARRAY[");
	for (size_t i = 0; i < tickers->n_tuples; i++)
		{
			tmp = mystrcat(tmp, "\'");
			tmp = mystrcat(tmp, (*(tickers->tickers))[i][1]);
			tmp = mystrcat(tmp, "\', ");
		}
	tmp -= 2;
	*tmp = '\0';
	tmp = strcat(tmp, "]) AS t(t_bbg, t_capiq) LEFT JOIN ciq.statements_standard_quarter AS b ON t_bbg = b.ticker_bbg AND t_capiq = b.ticker_capiq WHERE b.ticker_bbg IS NULL AND b.ticker_capiq IS NULL");
	conn = connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);

	// exec query
	res = PQexec(conn, request);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		{
			fprintf(stderr, "Can not obtain data from query\n");
			PQclear(res);
			PQfinish(conn);
			exit(2);
		}
	tickers->n_tuples = (size_t)PQntuples(res);
	tickers->n_cols = (size_t)PQnfields(res);
	for (size_t i = 0; i < tickers->n_tuples; i++)
		{
			for (size_t j = 0; j < 2; j++)
				{
					tmp = PQgetvalue(res, i, j);
					(*(tickers->tick_len))[i][j] = strlen(tmp) + 1;//count the null at end
					strcpy((*(tickers->tickers))[i][j], tmp);
				}
		}
	PQclear(res);
	PQfinish(conn);
}

int main(int argc, char **argv)
{
	server_t srv;
	client_t *cli_head;
	tickers_t tickers;
	uint64_t flags;
	uint64_t option_flags;
	int tmp_pos = 0;
	int opt;

	flags = 0x0;
	option_flags = 0x0;
	opterr = 0;
	while ((opt = getopt(argc, argv, "il:")) != -1)
		{
			switch (opt)
				{
				case 'i':
					option_flags |= F_INTERSECT;
					break;
				case 'l':
					tmp_pos = atoi(optarg);
					break;
				case '?':
					if (optopt == 'l')
						fprintf (stderr, "Option -%c requires an argument.\n", optopt);
					else if (optopt)
						fprintf (stderr, "Unknown option `-%c'.\n", optopt);
					else
						fprintf (stderr,
										 "Unknown option character `\\x%x'.\n",
										 optopt);
					return 1;
				default:
					abort ();
				}
		}
	tickers = (tickers_t){0, 0, tmp_pos, NULL, NULL, NULL};
	// if a header with the tickers is detected the tickers will be
	// loaded staticly in memory
#ifdef TICKERS_H
	char tickers_main[N_TUPLES][N_COLS][MAX_TICK_LEN] = TICKERS;
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
	if (option_flags & F_INTERSECT)
		intersect_tickers(&tickers);
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
