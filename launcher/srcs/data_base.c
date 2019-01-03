/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   data_base.c                                                              */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/03 11:05:18 by Mateo                                    */
/*   Updated: 2019/01/03 11:54:00 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"

int connect_db(const char *db_name,
							 const char *db_user,
							 const char *db_pass,
							 const char *db_host)
{
	PGconn *conn;
	int conn_end = 1;
	ConnStatusType conn_status = 0;
	ConnStatusType conn_status_tmp = 0;

	printf("Connecting to: %s (%s)", db_name, db_host);
	conn = PQsetdbLogin(db_host, NULL, NULL, NULL, db_name, db_user, db_pass);
	while (conn_end)
		{
			conn_status = PQstatus(conn);
			if (conn_status != conn_status_tmp)
				{
					switch(conn_status)
						{
						case CONNECTION_OK:
							printf("Connected to db\n");
							conn_end = 0;
								break;
						case CONNECTION_BAD:
							printf("Can not connect to db\n");
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
					conn_status = conn_status_tmp;
				}
		}
	return (1);
}
