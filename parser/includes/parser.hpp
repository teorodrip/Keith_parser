// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.hpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/08 19:02:25 by Mateo                                    //
//   Updated: 2019/01/25 18:08:08 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#ifndef PARSER_HPP
#define PARSER_HPP

#include <unistd.h>
#include </usr/include/postgresql/libpq-fe.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <regex>
#include "../../launcher/includes/protocol.h"

#define NAME_MAX 100
#define INOTIFY_BUFF (sizeof(struct inotify_event) + NAME_MAX + 1)
#define BATCH_SIZE 10
#define META_INFO_LEN 3
#define BUFF_SIZE 1024 //buffer must be greater than the length of the max ticker
#define FILE_NAME "./sheet2.xlsx"
#define COMPUER_NAME "unchartech_2"
#define DEFAULT_PATH "/home/" COMPUER_NAME "/vm_shared/outputs_windows_"

//parser macros
#define HIDEN_SHEET_1 "_CIQHiddenCacheSheet"
#define SHEET_0 "Key Stats"
#define SHEET_1 "Income Statements"
#define SHEET_2 "Balance Sheet"
#define SHEET_3 "Cash Flow"
#define SHEET_NB 4
#define SHEET_ARR {SHEET_0, SHEET_1, SHEET_2, SHEET_3}

//data base constants
#define DB_NAME "pam_test"
#define DB_USER "capiq_manager"
#define DB_PASS "capiqunchartech"
#define DB_HOST "192.168.27.122"
#define TABLE_PATH "ciq.statements_standard"
#define COL_BLOOM_TICKER "ticker_bbg"
#define COL_PERIOD_DATE "period_date"
#define COL_INC_FIL_DATE "income_filled_date"
#define COL_BAL_FIL_DATE "balance_filled_date"
#define COL_CASH_FIL_DATE "cashflow_filled_date"
#define COL_CAPIQ_TICKER "ticker_capiq"
#define COL_INC_STAT_SHEET "income_statement"
#define COL_BAL_SHEET "balance_sheet"
#define COL_CASH_SHEET "cash_flow"
#define COL_KEY_SHEET "key_stats"
//the array must follow the sheet order in the book
#define COLS_DATES {COL_PERIOD_DATE, COL_INC_FIL_DATE, COL_BAL_FIL_DATE, COL_CASH_FIL_DATE}
#define COLS_DATA {COL_KEY_SHEET, COL_INC_STAT_SHEET, COL_BAL_SHEET, COL_CASH_SHEET}


typedef struct ticker_json_s
{
  size_t len;
  nlohmann::json *j;
} ticker_json_t;

class data_base
{
private:
  PGconn *conn;
public:
  data_base();
  void connect_db(const char *db_name,
									const char *db_user,
									const char *db_pass,
									const char *db_host);
  bool upload_ticker_period(std::string capiq_ticker,
														std::string period_date,
														std::string bloom_ticker,
														std::string data,
														std::string date,
														unsigned char sheet_nb);
  void finish_db();
};


class dir_watcher
{
private:
  std::string path;
  int fd_notify;

public:
  dir_watcher();
  dir_watcher(const std::string path);
  char *watch_directory(char *name);
};


#endif
