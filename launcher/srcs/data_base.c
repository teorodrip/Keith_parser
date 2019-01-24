/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   data_base.c                                                              */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/03 11:05:18 by Mateo                                    */
/*   Updated: 2019/01/24 18:57:29 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"
#include <fcntl.h>

PGconn *connect_db(const char *db_name,
									 const char *db_user,
									 const char *db_pass,
									 const char *db_host)
{
	PGconn *conn;

	printf(F_LOG("Connecting to: %s (%s)\n"), db_name, db_host);
	conn = PQsetdbLogin(db_host, NULL, NULL, NULL, db_name, db_user, db_pass);
	switch(PQstatus(conn))
		{
		case CONNECTION_OK:
			printf("Connected to db\n");
			break;
		case CONNECTION_BAD:
			fprintf(stderr,"Can not connect to db\n");
			exit(2);
			break;
		case CONNECTION_AWAITING_RESPONSE:
			printf("Connection status: CONNECTION_AWAITING_RESPONSE\n");
			break;
		case CONNECTION_AUTH_OK:
			printf("Connection status: CONNECTION_AUTH_OK\n");
			break;
		case CONNECTION_CHECK_WRITABLE:
			printf("Connection status: CONNECTION_CHECK_WRITABLE\n");
			break;
		case CONNECTION_CONSUME:
			printf("Connection status: CONNECTION_CONSUME\n");
			break;
		case CONNECTION_MADE:
			printf("Connection status: CONNECTION_MADE\n");
			break;
		case CONNECTION_NEEDED:
			printf("Connection status: CONNECTION_NEEDED\n");
			break;
		case CONNECTION_SETENV:
			printf("Connection status: CONNECTION_SETENV\n");
			break;
		case CONNECTION_SSL_STARTUP:
			printf("Connection status: CONNECTION_SSL_STARTUP\n");
			break;
		case CONNECTION_STARTED:
			printf("Connection status: CONNECTION_STARTED\n");
			break;
		}
	return (conn);
}

void get_data(PGconn *conn, char *request, tickers_t *tickers)
{
	tickers->res = PQexec(conn, request);
	if (PQresultStatus(tickers->res) != PGRES_TUPLES_OK)
		{
			fprintf(stderr, "Can not obtain data from query\n");
			PQclear(tickers->res);
			PQfinish(conn);
			exit(2);
		}
	tickers->n_tuples = (size_t)PQntuples(tickers->res);
	tickers->n_cols = (size_t)PQnfields(tickers->res);
	if ( !(tickers->tick_len =
				 (unsigned char **)malloc(sizeof(unsigned char *) * tickers->n_tuples)))
		{
			dprintf(2, "Error: malloc in get_data\n");
			exit(EXIT_FAILURE);
		}
	for (size_t i = 0; i < tickers->n_tuples; i++)
		{
			if ( !(tickers->tick_len[i] =
						 (unsigned char *)malloc(sizeof(unsigned char) * tickers->n_cols)))
				{
					dprintf(2, "Error: malloc in get_data\n");
					exit(EXIT_FAILURE);
				}
			for (size_t j = 0; j < tickers->n_cols; j++)
				tickers->tick_len[i][j] = strlen(PQgetvalue(tickers->res, i, j)) + 1;//count the null at end
		}
}

void clean_tickers(tickers_t *tickers)
{
	for (uint32_t i = 0; i < tickers->n_tuples; i++)
		free(tickers->tick_len[i]);
	free(tickers->tick_len);
}
