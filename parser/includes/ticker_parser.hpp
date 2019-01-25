// ************************************************************************** //
//                                                                            //
//                                                                            //
//   ticker_parser.hpp                                                        //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/25 14:37:56 by Mateo                                    //
/*   Updated: 2019/01/25 18:53:04 by Mateo                                    */
//                                                                            //
// ************************************************************************** //

#ifndef TICKPARS_HPP
#define TICKPARS_HPP

#include <nlohmann/json.hpp>
#include <xlsxio_read.h>
#include "../includes/error_handler.hpp"

#define N_SHEETS 4

#define TICKER_START "Ticker/ID"
#define HALF_TICKER "HALF_OF_TICKER"
#define END_TICKER "END_OF_TICKER"
#define FIL_DATE "Filing Date"
#define F_START 0x1
#define F_HALF 0x2
#define F_END_TICKER 0x4
#define F_END_PARSING 0x8
#define F_FIL_DATE 0x10
#define F_ERROR_IN_TICKER 0x20
#define F_FATAL_ERROR 0x40

typedef struct ticker_data_s
{
	std::string ticker_bbg;
	std::string period_date;
	std::string capiq_ticker;
	std::vector<std::string> income_filled_date;
	std::vector<std::string> balance_filled_date;
	std::vector<std::string> cashflow_filled_date;
	std::vector<nlohmann::json> income_statement;
	std::vector<nlohmann::json> balance_sheet;
	std::vector<nlohmann::json> cash_flow;
	std::vector<nlohmann::json> key_stats;
} ticker_data_t;

class ticker_parser
{
private:
	ticker_data_t data;
	unsigned int flags;
	error_handler *err;
	size_t ticker_index;

public:
	ticker_parser();

	ticker_parser(error_handler *err);
	unsigned char parse_ticker(const xlsxioreadersheet *sheets, const size_t len);
};

#endif
