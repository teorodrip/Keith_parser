// ************************************************************************** //
//                                                                            //
//                                                                            //
//   data_base.cpp                                                            //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/18 15:27:33 by Mateo                                    //
//   Updated: 2019/02/07 13:47:49 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <fstream>

//constructor
data_base::data_base()
{
  conn = NULL;
}

// connect to the data base specified in the header file
void data_base::connect_db(const char *db_name,
													 const char *db_user,
													 const char *db_pass,
													 const char *db_host)
{
  printf("Connecting to: %s (%s)\n", db_name, db_host);
  conn = PQsetdbLogin(db_host, NULL, NULL, NULL, db_name, db_user, db_pass);
	// switch betweeen possible errors
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

// parse the date from excel format to SQL format
static std::string parse_excel_date(std::string date_str)
{
	int n_day, n_month, n_year;
	std::string parsed_date;
	int date;
	// try to convert the string to number, if is not a valid number like "NA" the date is set to 0
	try
		{
			date = std::stoi(date_str);
		}
	catch (std::exception &e)
		{
			printf("Warning: Data with value: %s, this date is going to be set to 0 (Excel format)\n", date_str.c_str());
			date = 0;
		}
	// Just magic tricks
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

bool data_base::upload_ticker_daily(const std::string *values)
{
	std::string request("INSERT INTO " TABLE_DAILY_PATH " VALUES " + *values);
	if (exec_query(request))
		exec_query("ROLLBACK");
	return (false);
}

// upload a ticker quarter and year to the data base
bool data_base::upload_ticker(ticker_json_t *tick, std::string bloom_ticker, sheet_t *sheets)
{
	std::string request;
  std::string request_base;
  std::string request_body = "";
	size_t pos;

	// header of transaction
	request = "BEGIN TRANSACTION;\n";
	request_base = "INSERT INTO " TABLE_YEAR_PATH " VALUES";
	// fill the year row
	for (size_t i = 1; i < tick->dates_year->size(); i++)
		{
			if (i > 1)
				request_body += ",";
			request_body += "(\'" + bloom_ticker +
				"\',\'" + parse_excel_date(tick->dates_year->at(i)) + "\',";
			//fil date 1
			pos = sheets[1].fil_date[sheets[1].fil_date_iter - 2].i;
			if (i < sheets[1].sheet[pos].size())
				request_body += "\'" + parse_excel_date(sheets[1].sheet[pos][i]) + "\',";
			else
				request_body += "NULL,";
			//fill date 2
			pos = sheets[2].fil_date[sheets[2].fil_date_iter - 2].i;
			if (i < sheets[2].sheet[pos].size())
				request_body += "\'" + parse_excel_date(sheets[2].sheet[pos][i]) + "\',";
			else
				request_body += "NULL,";
			//fill date 3
			pos = sheets[3].fil_date[sheets[3].fil_date_iter - 2].i;
			if (i < sheets[3].sheet[pos].size())
				request_body += "\'" + parse_excel_date(sheets[3].sheet[pos][i]) + "\',";
			else
				request_body += "NULL,";
			request_body += "\'" + tick->ticker_capiq +
				"\',\'" + tick->j_year[1][i - 1].dump() +
				"\',\'" + tick->j_year[2][i - 1].dump() +
				"\',\'" + tick->j_year[3][i - 1].dump() +
				"\',\'" + tick->j_year[0][i - 1].dump() +
				"\')";
		}
	// the other insert
	request += request_base + request_body + ";\n";
	request_base = "INSERT INTO " TABLE_QUARTER_PATH " VALUES";
	request_body = "";
	//	fill the quarter row
	for (size_t i = 1; i < tick->dates_quarter->size(); i++)
		{
			if (i > 1)
				request_body += ",";
			request_body += "(\'" + bloom_ticker +
				"\',\'" + parse_excel_date(tick->dates_quarter->at(i)) + "\',";
			//fil date 1
			pos = sheets[1].fil_date[sheets[1].fil_date_iter - 1].i;
			if (i < sheets[1].sheet[pos].size())
				request_body += "\'" + parse_excel_date(sheets[1].sheet[pos][i]) + "\',";
			else
				request_body += "NULL,";
			//fill date 2
			pos = sheets[2].fil_date[sheets[2].fil_date_iter - 1].i;
			if (i < sheets[2].sheet[pos].size())
				request_body += "\'" + parse_excel_date(sheets[2].sheet[pos][i]) + "\',";
			else
				request_body += "NULL,";
			//fill date 3
			pos = sheets[3].fil_date[sheets[3].fil_date_iter - 1].i;
			if (i < sheets[3].sheet[pos].size())
				request_body += "\'" + parse_excel_date(sheets[3].sheet[pos][i]) + "\',";
			else
				request_body += "NULL,";
			request_body += "\'" + tick->ticker_capiq +
				"\',\'" + tick->j_quarter[1][i - 1].dump() +
				"\',\'" + tick->j_quarter[2][i - 1].dump() +
				"\',\'" + tick->j_quarter[3][i - 1].dump() +
				"\',\'" + tick->j_quarter[0][i - 1].dump() +
				"\')";
		}
	// commit the transaction
	request += request_base + request_body + ";\n" + " COMMIT;";
	// execute the transaction, if something went wrong ROLLBACK to continue with the other tickers
	if (exec_query(request))
		exec_query("ROLLBACK");
	return (false);
}

// execute a query
bool data_base::exec_query(std::string query)
{
  PGresult *res;
	char *error_message;
	std::regex err_check (".*duplicate.*");

  res = PQexec(conn, query.c_str());
	// if the query is not well execute return error, and print error
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
		{
			error_message = PQresultErrorMessage(res);
			printf(SEPARATOR "%s" SEPARATOR, error_message);
			if (!(std::regex_search(error_message, err_check, std::regex_constants::match_any)))
				{
					PQclear(res);
					PQfinish(conn);
					exit(EXIT_FAILURE);
				}
			PQclear(res);
			return (true);
		}
  PQclear(res);
	return (false);
}

// end the connection
void data_base::finish_db()
{
  PQfinish(conn);
}
