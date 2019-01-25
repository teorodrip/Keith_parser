// ************************************************************************** //
//                                                                            //
//                                                                            //
//   error_handler.hpp                                                        //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/25 14:26:32 by Mateo                                    //
//   Updated: 2019/01/25 18:22:09 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#ifndef ERRHAND_HPP
#define ERRHAND_HPP

#include <stdlib.h>
#include <string>
#include <vector>

#define BATCH_SIZE 10
#define N_ERRORS 3
#define CIQINACTIVE_RTETRIES 4
#define INVALID_IDNENTIFIER_RETRIES 1
#define REFRESH_RETRIES 1
#define ERR_CIQ "#CIQINACTIVE"
#define ERR_INV "(Invalid Identifier)"
#define ERR_REF "#REFRESH"
#define ERROR_ARR {ERR_CIQ, ERR_INV, ERR_REF}
#define RETRY_ARR {CIQINACTIVE_RTETRIES, INVALID_IDNENTIFIER_RETRIES, REFRESH_RETRIES}

#define E_REBOOT 0x0
#define E_ERROR_IN_TICKER 0x1
#define E_NO_ERRORS 0x2

typedef unsigned char retries_t[N_ERRORS];

typedef struct queue_s
{
  size_t start; //inclusive
  size_t end; //exclusive
  struct queue_s *next;
} queue_t;

class error_handler
{
private:
	retries_t *ticker_retries;
	std::vector<queue_t> queue;

	void add_to_queue(const size_t ticker_index);

public:
	error_handler();
	unsigned char check_cell(const std::string cell_value, const size_t ticker_index);
};

#endif
