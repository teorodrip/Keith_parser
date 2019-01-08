/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   data_base.c                                                              */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/03 11:05:18 by Mateo                                    */
/*   Updated: 2019/01/07 15:27:20 by Mateo                                    */
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

	printf("Connecting to: %s (%s)\n", db_name, db_host);
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
	PGresult *res;

	res = PQexec(conn, request);
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
		{
			fprintf(stderr, "Can not obtain data from query\n");
			PQclear(res);
			PQfinish(conn);
			exit(2);
		}
	tickers->len = (uint32_t)PQntuples(res);
	printf("Got %u tickers\n", tickers->len);
	if (!(tickers->tickers = (char **)malloc(sizeof(char *) * tickers->len)))
		{
			dprintf(2, "Error: in malloc get_data\n");
			exit(EXIT_FAILURE);
		}
	for (uint32_t i = 0; i < tickers->len; i++)
			tickers->tickers[i] = strdup(PQgetvalue(res, i, 0));
	PQclear(res);
}

void write_tickers(PGresult *res, char *path)
{
	int fd;
	char buff[WRITE_BUFF];
	char *ticker;
	char file_path[NAME_MAX];
	int 	n_tuples;
	int i;
	size_t batch_counter;
	size_t file_counter;
	size_t ticker_len;
	size_t buff_pos;

	buff_pos = 0;
	i = 0;
	ticker = PQgetvalue(res, i, 0);
	n_tuples = PQntuples(res);
	file_counter = 0;
	batch_counter = 0;
	sprintf(file_path, "%stickers_%lu.dat", path, file_counter);
	if ((fd = open(file_path, O_TRUNC | O_WRONLY | O_CREAT, 0777)) < 0)
		{
			fprintf(stderr, "Error opening the file: %s", file_path);
			exit(2);
		}
	while (i < n_tuples)
		{
			ticker_len = strlen(ticker);
			if ((buff_pos + 1 + ticker_len) <= WRITE_BUFF)//1 for \n
				{
					strncpy(buff + buff_pos, ticker, ticker_len);
					buff_pos += ticker_len;
					strncpy(buff + buff_pos++, "\n", 1);
					if (++batch_counter == BATCH_SIZE)
						{
							write(fd, buff, buff_pos);
							close(fd);
							batch_counter = 0;
							buff_pos = 0;
							memset(buff, 0, WRITE_BUFF);
							sprintf(file_path, "%stickers_%lu.dat", path, ++file_counter);
							if ((fd = open(file_path, O_TRUNC | O_WRONLY | O_CREAT, 0777)) < 0)
								{
									fprintf(stderr, "Error opening the file: %s", file_path);
									exit(2);
								}
						}
					if (++i < n_tuples)
						{
							ticker = PQgetvalue(res, i, 0);
						}
					else
						write(fd, buff, buff_pos);
				}
			else
				{
					strncpy(buff + buff_pos, ticker, WRITE_BUFF - buff_pos);
					ticker += WRITE_BUFF - buff_pos;
					write(fd, buff, WRITE_BUFF);
					buff_pos = 0;
					memset(buff, 0, WRITE_BUFF);
				}
		}
	close(fd);
}

void clean_tickers(tickers_t *tickers)
{
	for (uint32_t i = 0; i < tickers->len; i++)
		free(tickers->tickers[i]);
	free(tickers->tickers);
}
