// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.hpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/08 19:02:25 by Mateo                                    //
//   Updated: 2019/01/31 19:00:52 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#ifndef PARSER_HPP
#define PARSER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <xlsxio_read.h>
#include <unistd.h>
#include </usr/include/postgresql/libpq-fe.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
#include <regex>
#include "../../launcher/includes/protocol.h"

#define CIQINACTIVE_RTETRIES 4
#define INVALID_IDNENTIFIER_RETRIES 1
#define REFRESH_RETRIES 1
#define ERR_CIQ "#CIQINACTIVE"
#define ERR_INV "(Invalid Identifier)"
#define ERR_REF "#REFRESH"
#define ERROR_ARR {ERR_CIQ, ERR_INV, ERR_REF, ""}
#define RETRY_ARR {CIQINACTIVE_RTETRIES, INVALID_IDNENTIFIER_RETRIES, REFRESH_RETRIES, -1}

#define NAME_MAX 100
#define INOTIFY_BUFF (sizeof(struct inotify_event) + NAME_MAX + 1)
#define BATCH_SIZE 10
#define META_INFO_LEN 3
#define PORT 8080
#define ADDR INADDR_ANY
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
#define TICKER_START "Ticker/ID"
#define HALF_TICKER "HALF_OF_TICKER"
#define END_TICKER "END_OF_TICKER"
#define FIL_DATE "Filing Date"
#define F_START 0x1
#define F_HALF 0x2
#define F_END 0x4
#define F_END_PARSING 0x8
#define F_FIL_DATE 0x10
#define F_ERROR_IN_TICKER 0x20
#define F_FATAL_ERROR 0x40

//data base constants
#define DB_NAME "pam_test"
#define DB_USER "capiq_manager"
#define DB_PASS "capiqunchartech"
#define DB_HOST "192.168.27.122"
#define TABLE_YEAR_PATH "ciq.statements_standard_year"
#define TABLE_QUARTER_PATH "ciq.statements_standard_quarter"
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

typedef struct queue_s
{
  uint32_t start; //inclusive
  uint32_t end; //exclusive
  struct queue_s *next;
} queue_t;

typedef struct ticker_json_s
{
	size_t ticker_index;
	std::string ticker_capiq;
	std::vector<std::string> *dates_year;
	std::vector<std::string> *dates_quarter;
	std::vector<nlohmann::json> j_year[SHEET_NB];
	std::vector<nlohmann::json> j_quarter[SHEET_NB];
} ticker_json_t;

typedef struct coord_s
{
	size_t i;
	size_t j;
} coord_t;

#define FS_END_OF_SHEET 0x1
typedef struct sheet_s
{
	int flags;
	std::vector<coord_t> ticker_id;
	std::vector<coord_t> end_tick;
	std::vector<coord_t> fil_date;
	size_t ticker_id_iter;
	size_t end_ticker_iter;
	size_t fil_date_iter;
	std::vector<std::vector<std::string>> sheet;
} sheet_t;

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
  bool upload_ticker(ticker_json_t *tick, std::string bloom_ticker, sheet_t *sheets);
  void finish_db();
};

class client
{
private:
  struct sockaddr_in serv_addr;
  int sockfd;
  char **init_bloom_tick(unsigned short *n_tickers, const char *buff, const int readed, unsigned int *buff_pos);

public:
  client();
  void init();
  char **get_tickers(unsigned short *n_tickers);
  unsigned char get_watching_directories();
  void signal_shutdown(const unsigned char vm_nb);
  void signal_reboot(const unsigned char vm_nb);
  void send_queue(const queue_t queue);
	void signal_parsing();
	void signal_end_parsing();
	unsigned char check_end();
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

class excel_parser : public client, public data_base
{
private:
  char **bloom_tickers;
	int *ticker_retries;
	bool *tickers_in_queue;
	sheet_t sheets[SHEET_NB];
  unsigned short n_bloom_tickers;
  std::string file_path;
  xlsxioreader book;
  std::string sheet_names[SHEET_NB];
  unsigned char vm_id;
	queue_t queue;
  int flags;

  bool issdigit(char *str);
  void handle_fatal_error(const std::string message);
	bool handle_cell_error(std::string value);
  std::string parse_excel_date(int serial_date);
	bool mark_cell_error(std::string cell_value, sheet_t *sheet);
	void parse_tickers();

public:
  excel_parser();
	void init();
  void load_book(const std::string file_path);
  void parse_book();
  void clear_flags();
  void close_book();
  void clear_all();
  void free_all();
	void clear_bloom_tickers();
	void clear_ticker_retries();
	void clear_period_dates();
};

#endif
