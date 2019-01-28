// ************************************************************************** //
//                                                                            //
//                                                                            //
//   excel_reader.hpp                                                         //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/25 11:32:03 by Mateo                                    //
//   Updated: 2019/01/28 12:26:08 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#ifndef EXREAD_HPP
#define EXREAD_HPP

#include <xlsxio_read.h>
#include <vector>
#include <string>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include "../../launcher/includes/logger.h"
#include "../includes/ticker_parser.hpp"
#include "../includes/error_handler.hpp"

typedef size_t sheet_coord_t[2];

typedef struct sheet_s
{
	std::vector<sheet_coord_t> marks;
	std::vector<std::vector<std::string>> sheet;
} sheet_t;

class excel_reader
{
private:
  // char **bloom_tickers;
	// int *ticker_retries;
	// std::string *period_dates;
	// bool *tickers_in_queue;
  // unsigned short n_bloom_tickers;
  // std::string file_path;
  // std::string ticker_name;
	// std::string fil_date;
	// unsigned char current_sheet;
  // size_t ticker_index;
  // unsigned char vm_id;
	// queue_t queue;
  // int flags;


  // bool issdigit(char *str);
  // void handle_fatal_error(const std::string message);
	// bool handle_cell_error(std::string value);
  // void parse_row(const xlsxioreadersheet sheet, ticker_json_t *j_quarter, ticker_json_t *j_year);
  // void init_index(const xlsxioreadersheet sheet);
  // void init_ticker(const xlsxioreadersheet sheet);
  // void init_date(const xlsxioreadersheet sheet, ticker_json_t *j);
  // bool jump_rows(const xlsxioreadersheet sheet, const size_t cuant);
  // std::string parse_excel_date(int serial_date);
	// void mark_cell_error(std::string cell_value);
protected:
  xlsxioreader book;
	std::vector<std::string> sheet_names;
	size_t sheet_nb;
	std::vector<sheet_t> sheets;

public:
  excel_reader();
	// void init();
  xlsxioreader load_book(const std::string file_path,
												 const std::regex *sheet_exceptions,
												 const size_t regex_siz);
	std::vector<sheet_t> load_sheets(const std::regex *sheet_marks,
																	 const size_t regex_siz);
	std::string get_value(const size_t sheet_pos, const size_t row, const size_t col);
	void close_book();
	// void parse_book();
	// void clear_flags();
	// void close_book();
	// void clear_all();
	// void clear_bloom_tickers();
	// void clear_ticker_retries();
	// void clear_period_dates();
};


#endif
