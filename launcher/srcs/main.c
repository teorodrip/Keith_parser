/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   main.c                                                                   */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/02 14:21:03 by Mateo                                    */
/*   Updated: 2019/01/07 15:01:41 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"
#include "../includes/server.h"

int main()
{
	PGconn *conn;
	PGresult *res;
	server_t srv;
	client_t *cli;
	tickers_t tickers;
	uint32_t pos;

	conn = connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
	get_data(conn, SQL_ALL_REQ, &tickers);
	PQfinish(conn);
	init_server(&srv);
	cli = NULL;
	pos = 0;
	while (pos < tickers.len)
		{

		}
}
