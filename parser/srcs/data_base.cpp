// ************************************************************************** //
//                                                                            //
//                                                                            //
//   data_base.cpp                                                            //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/18 15:27:33 by Mateo                                    //
//   Updated: 2019/01/30 19:21:15 by Mateo                                    //
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

static std::string parse_excel_date(std::string date_str)
{
	int n_day, n_month, n_year;
	std::string parsed_date;
	int date = std::stoi(date_str);
	// Excel/Lotus 123 have a bug with 29-02-1900. 1900 is not a
	// leap year, but Excel/Lotus 123 think it is...
	if (date == 60)
    {
			n_day    = 29;
			n_month    = 2;
			n_year    = 1900;
    }
	else
		{
			if (date < 60)
				{
					// Because of the 29-02-1900 bug, any serial date
					// under 60 is one off... Compensate.
					date++;
				}
			// Modified Julian to DMY calculation with an addition of 2415019
			int l = date + 68569 + 2415019;
			int n = int(( 4 * l ) / 146097);
			l = l - int(( 146097 * n + 3 ) / 4);
			int i = int(( 4000 * ( l + 1 ) ) / 1461001);
			l = l - int(( 1461 * i ) / 4) + 31;
			int j = int(( 80 * l ) / 2447);
			n_day = l - int(( 2447 * j ) / 80);
			l = int(j / 11);
			n_month = j + 2 - ( 12 * l );
			n_year = 100 * ( n - 49 ) + i + l;
		}
	parsed_date = std::to_string(n_day) + "-" + std::to_string(n_month) +
		"-" + std::to_string(n_year);
	return (parsed_date);
}

bool data_base::upload_ticker(ticker_json_t *tick, std::string bloom_ticker, sheet_t *sheets)
{
  PGresult *res;
  std::string request_base;
  std::string request_body_year = "";
	size_t pos;

	request_base = "INSERT INTO " TABLE_PATH " VALUES";
	for (size_t i = 1; i < tick->dates_year->size(); i++)
		{
			if (i > 1)
				request_body_year += ",";
			request_body_year += "(\'" + bloom_ticker +
				"\',\'" + parse_excel_date(tick->dates_year->at(i)) + "\',";
			//fil date 1
			pos = sheets[1].fil_date[sheets[1].fil_date_iter - 2].i;
			if (i < sheets[1].sheet[pos].size())
				request_body_year += "\'" + parse_excel_date(sheets[1].sheet[pos][i]) + "\',";
			else
				request_body_year += "NULL,";
			//fill date 2
			pos = sheets[2].fil_date[sheets[2].fil_date_iter - 2].i;
			if (i < sheets[2].sheet[pos].size())
				request_body_year += "\'" + parse_excel_date(sheets[2].sheet[pos][i]) + "\',";
			else
				request_body_year += "NULL,";
			//fill date 3
			pos = sheets[3].fil_date[sheets[3].fil_date_iter - 2].i;
			if (i < sheets[3].sheet[pos].size())
				request_body_year += "\'" + parse_excel_date(sheets[3].sheet[pos][i]) + "\',";
			else
				request_body_year += "NULL,";
			request_body_year += "\'" + tick->ticker_capiq +
				"\',\'" + tick->j_year[1][i - 1].dump() +
				"\',\'" + tick->j_year[2][i - 1].dump() +
				"\',\'" + tick->j_year[3][i - 1].dump() +
				"\',\'" + tick->j_year[0][i - 1].dump() +
				"\')";
		}
	request_base += request_body_year;
  res = PQexec(conn, request_base.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
		{
			std::cout << "Error: executing the following request:\n" + request_base + "\n";
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
