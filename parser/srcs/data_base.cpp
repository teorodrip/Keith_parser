// ************************************************************************** //
//                                                                            //
//                                                                            //
//   data_base.cpp                                                            //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/18 15:27:33 by Mateo                                    //
//   Updated: 2019/01/22 15:14:27 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <fstream>

data_base::data_base()
{
  conn = NULL;
}

void data_base::connect_db(const char *db_name,
						   const char *db_user,
						   const char *db_pass,
						   const char *db_host)
{
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
}

bool data_base::upload_ticker_period(std::string capiq_ticker,
									 std::string period_date,
									 std::string bloom_ticker,
									 std::string data,
									 std::string date,
									 unsigned char sheet_nb)
{
  PGresult *res;
  std::string request;
  std::string date_columns[SHEET_NB] = COLS_DATES;
  std::string data_columns[SHEET_NB] = COLS_DATA;

  if (sheet_nb == 0)
	{
	  request = "INSERT INTO " TABLE_PATH " VALUES(\'" + bloom_ticker +
		"\',\'" + period_date + "\',NULL,NULL,NULL,\'" + capiq_ticker +
		"\',NULL,NULL,NULL,\'" + data + "\');";
	}
  else if (sheet_nb < SHEET_NB)
	{
	  request = "UPDATE " TABLE_PATH " SET " + date_columns[sheet_nb] +
		" = \'" + date + "\', " + data_columns[sheet_nb] + " = \'" +
		data + "\' WHERE " COL_BLOOM_TICKER " = \'" + bloom_ticker +
		"\' AND " COL_PERIOD_DATE " = \'" + period_date + "\';";
	}
  res = PQexec(conn, request.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
	  std::cerr << "Error: executing the following request:\n" + request + "\n";
	  PQclear(res);
		return (true);
	}
  // res = PQexec(conn, "COMMIT");
  // if (PQresultStatus(res) != PGRES_COMMAND_OK)
	// {
	//   std::cerr << "Error: executing the following request:\nCOMMIT\n";
	//   PQclear(res);
	//   PQfinish(conn);
	//   exit(EXIT_FAILURE);
	// }
  PQclear(res);
	return (false);
}

void data_base::finish_db()
{
  PQfinish(conn);
}
