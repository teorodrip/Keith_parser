// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.hpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/08 19:02:25 by Mateo                                    //
//   Updated: 2019/01/18 10:56:43 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#ifndef PARSER_HPP
#define PARSER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <xlsxio_read.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

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
#define TICKER_START "Ticker/ID"
#define HALF_TICKER "HALF_OF_TICKER"
#define END_TICKER "END_OF_TICKER"
#define F_START 0x1
#define F_HALF 0x2
#define F_END 0x4
#define F_END_PARSING 0x8

typedef struct queue_s
{
	uint32_t start; //inclusive
	uint32_t end; //exclusive
	struct queue_s *next;
} queue_t;

typedef struct ticker_json_s
{
	size_t len;
	nlohmann::json *j;
} ticker_json_t;

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

class excel_parser : public client
{
private:
	std::string file_path;
	xlsxioreader book;
	std::vector<std::string> sheet_names;
	size_t ticker_index;
	unsigned char vm_id;
	int flags;

	bool issdigit(char *str);
	void handle_fatal_error(const std::string message);
	void handle_cell_error(size_t n_tuples, std::string value);
	void parse_row(const xlsxioreadersheet sheet, ticker_json_t *j_quarter, ticker_json_t *j_year);
	void init_ticker(const xlsxioreadersheet sheet, ticker_json_t *j);
	void init_date(const xlsxioreadersheet sheet, ticker_json_t *j);

public:
	excel_parser();
	void init(const std::string file_path);
	void parse_book();
	void set_file_path(const std::string file_path);
	void clear_flags();
	void close_book();
	void clear_all();
};

#endif
