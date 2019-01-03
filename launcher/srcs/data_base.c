/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   data_base.c                                                              */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/03 11:05:18 by Mateo                                    */
/*   Updated: 2019/01/03 17:33:20 by Mateo                                    */
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

PGresult *get_data(PGconn *conn, char *request)
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
	return (res);
}

void write_tickers(PGresult *res, char *path)
{
	int fd;
	char buff[WRITE_BUFF];
	char *ticker;
	int 	n_tuples;
	int i;
	size_t ticker_len;
	size_t buff_pos;

	if ((fd = open(path, O_TRUNC | O_WRONLY)) < 0)
		{
			fprintf(stderr, "Error opening the file: %s", path);
			exit(2);
		}
	buff_pos = 0;
	i = 0;
	ticker = PQgetvalue(res, i, 0);
	n_tuples = PQntuples(res);
	while (i < n_tuples)
		{
			ticker_len = strlen(ticker);
			if ((buff_pos + 1 + ticker_len) <= WRITE_BUFF)//1 for \n
				{
					strncpy(buff + buff_pos, ticker, ticker_len);
					buff_pos += ticker_len;
					strncpy(buff + buff_pos, "\n", 1);
					buff_pos++;
					if (++i < n_tuples)
						ticker = PQgetvalue(res, i, 0);
					else
						write(fd, buff, buff_pos + 1);
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
