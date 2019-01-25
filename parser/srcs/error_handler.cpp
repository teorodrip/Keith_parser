// ************************************************************************** //
//                                                                            //
//                                                                            //
//   error_handler.cpp                                                        //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/25 14:26:13 by Mateo                                    //
//   Updated: 2019/01/25 18:26:42 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/error_handler.hpp"

error_handler::error_handler()
{
	ticker_retries = NULL;
}

void error_handler::add_to_queue(const size_t ticker_index)
{
	if (!queue.empty() && (ticker_index - queue.back().end) == 0 &&
			(queue.back().end - queue.back().start) < BATCH_SIZE)
			queue.back().end++;
	else
		queue.push_back({ticker_index, ticker_index + 1, NULL});
}

unsigned char error_handler::check_cell(const std::string cell_value, const size_t ticker_index)
{
	std::string error_arr[N_ERRORS] = ERROR_ARR;
	char retry_arr[N_ERRORS] = RETRY_ARR;

	for (size_t i = 0; i < N_ERRORS; i++)
		{
			if (ticker_retries[ticker_index][i] >= retry_arr[i])
				{
					//reboot
					return(E_REBOOT);
				}
		}
	for (size_t i = 0; i < N_ERRORS; i++)
		{
			if (cell_value == error_arr[i])
				{
					ticker_retries[ticker_index][i]++;
					add_to_queue(ticker_index);
					return (E_ERROR_IN_TICKER);
				}
		}
	return (E_NO_ERRORS);
}
