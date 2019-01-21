// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.cpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/10 17:57:13 by Mateo                                    //
//   Updated: 2019/01/21 19:21:40 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <stdlib.h>

excel_parser::excel_parser() : client(), data_base()
{
  this->bloom_tickers = NULL;
  this->file_path = "";
  this->flags = 0x0;
  this->ticker_index = 0;
  this->vm_id = 0;
  this->sheet_names = {};
  this->dates = {};
}

void excel_parser::handle_fatal_error(const std::string message)
{
  std::cerr << "Fatal error: " << message << "\n";
  exit(EXIT_FAILURE);
}

bool excel_parser::issdigit(char *str)
{
  if (!*str)
	return (false);
  while (*str)
	{
	  if (!std::isdigit(*str))
		return (false);
	  str++;
	}
  return (true);
}

void excel_parser::init(const std::string file_path)
{
  this->file_path = file_path;
  if ((book = xlsxioread_open(file_path.c_str())) == NULL)
	{
	  std::cerr << "Error opening: " << file_path << "\n";
	  exit(EXIT_FAILURE);
	}
  //list available sheets
  xlsxioreadersheetlist sheetlist;
  const XLSXIOCHAR* sheetname;
  if ((sheetlist = xlsxioread_sheetlist_open(book)) != NULL)
	{
	  while ((sheetname = xlsxioread_sheetlist_next(sheetlist)) != NULL)
		sheet_names.push_back(sheetname);
	  xlsxioread_sheetlist_close(sheetlist);
	}
  else
	{
	  std::cerr << "Error: listing the sheets\n";
	  exit(EXIT_FAILURE);
	}
	this->bloom_tickers = client::get_tickers(&n_bloom_tickers);
	printf("Got %d tickers\n", n_bloom_tickers);
	data_base::connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
}

void excel_parser::parse_book()
{
  xlsxioreadersheet sheet;
  ticker_json_t j_quarter;//before half ticker
  ticker_json_t j_year;//after half ticker
  unsigned char sheet_counter;

  sheet_counter = 0;
  for (std::string sheet_name : sheet_names)
	{
	  if ((sheet = xlsxioread_sheet_open(book, sheet_name.c_str(),
										 XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL)
		{
		  j_quarter = {0, NULL};
		  j_year = {0, NULL};
		  while (!(this->flags & F_END_PARSING) && xlsxioread_sheet_next_row(sheet))
			{
			  parse_row(sheet, &j_quarter, &j_year, sheet_counter);
			}
		  xlsxioread_sheet_close(sheet);
		  delete[] j_quarter.j;
		  delete[] j_year.j;
		}
	  break;
	  sheet_counter++;
	}
}

void excel_parser::parse_row(const xlsxioreadersheet sheet, ticker_json_t *j_quarter,
							 ticker_json_t *j_year, unsigned char sheet_counter)
{
  ticker_json_t *j;
  size_t cell_j;
  char *cell_value;
  std::string current_field;

  cell_j = 0;
  j = (this->flags & F_HALF) ? j_year : j_quarter;
  while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL)
	{
	  if (cell_j == 0)
		{
		  if (!strcmp(cell_value, TICKER_START) && !(flags & F_START))
			{
			  this->flags = 0x0;
			  init_ticker(sheet);
			  jump_rows(sheet, 1); //next row contains index
			  init_index(sheet);
			  jump_rows(sheet, 2); //jump company name and go to dates rows
			  init_date(sheet, j);
			  return; //continue parsing for next row
			}
		  else if (!strcmp(cell_value, HALF_TICKER) && (flags & F_START) &&
				   !(flags & F_HALF))
			{
			  this->flags |= F_HALF;
			  jump_rows(sheet, 3); //go to dates row
			  j = j_year; //reached half of ticker
			  init_date(sheet, j);
			  return; //continue for next row
			}
		  else if (!strcmp(cell_value, END_TICKER) && (flags & F_START) &&
				   (flags & F_HALF) && !(flags & F_END))
			{
			  // not add the flag but set flags to end so the others are cleared
			  this->flags = F_END;
			  jump_rows(sheet, 1); //jump empty row after end
			  //upload to data base
			  data_base::upload_ticker_period(ticker_name, dates[0],
											  bloom_tickers[ticker_index],
											  j->j->dump(),
											  dates[sheet_counter],
											  sheet_counter);
			  this->dates.clear();
			  data_base::finish_db();
			  exit(1);
			  return;
			}
		  else if (*cell_value == '\0')
			{
			  if (this->flags & F_END)
				this->flags = F_END_PARSING;
			  return; // if not ended parsing jump row
			}
		  else
			{
			  if (!strcmp(cell_value, FIL_DATE))
				this->flags |= F_FIL_DATE;
			  current_field = cell_value;
			}
		}
	  else if (*cell_value == '\0' || (cell_j - 1) >= j->len)
		break;
	  else
		(j->j[cell_j - 1])[current_field] = cell_value;
	  if (this->flags & F_FIL_DATE)
		{
		  this->dates.push_back(cell_value);
		  this->flags &= ~(F_FIL_DATE);
		}
	  cell_j++;
	}
}

void excel_parser::handle_cell_error(size_t n_tuples, std::string value)
{
  static int *ticker_retries = new int[n_tuples]();
  static queue_t queue = {0, 0, NULL};
  std::string error_arr[] = ERROR_ARR;
  char retry_arr[] = RETRY_ARR;
  unsigned char i = 0;
  int error_mask = 0xFF;

  while(retry_arr[i] >= 0)
	{
	  if ((((error_mask << (8 * i)) & ticker_retries[this->ticker_index]) >> (8 * i)) >=
		  retry_arr[i])
		{
		  //reboot vm;
		  client::signal_reboot(this->vm_id);
		}
	}
  i = 0;
  while (!error_arr[i].empty())
	{
	  if (value == error_arr[i])
		{
		  if (!queue.end)
			{
			  queue.start = ticker_index;
			  queue.end = ticker_index + 1;
			}
		  else if ((ticker_index - queue.end) == 0 &&
				   (queue.end - queue.start) < BATCH_SIZE)
			queue.end++;
		  else
			{
			  //send queue;
			  client::send_queue(queue);
			  queue.start = ticker_index;
			  queue.end = ticker_index + 1;
			}
		}
	}
}

//this will create the array of json for each date of the ticker
void excel_parser::init_date(const xlsxioreadersheet sheet, ticker_json_t *j)
{
  char *cell_value;

  j->len = (size_t)-1; //the first cell does not count
  delete[] j->j;
  while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL && *cell_value != '\0')
	{
	  if (this->dates.empty())
		this->dates.push_back(cell_value);
	  j->len++;
	}
  j->j = new nlohmann::json[j->len];
}

void excel_parser::init_ticker(const xlsxioreadersheet sheet)
{
  char *cell_value;

  //ticker must be in this line second cell
  if ((cell_value = xlsxioread_sheet_next_cell(sheet)) &&
	  *cell_value != '\0')
	this->ticker_name = cell_value;
  else
	handle_fatal_error("In the format of the sheet while reading ticker");
}

void excel_parser::init_index(const xlsxioreadersheet sheet)
{
  char *cell_value;

  //index must be in this line
  if ((cell_value = xlsxioread_sheet_next_cell(sheet)) &&
	  this->issdigit(cell_value))
	{
	  this->ticker_index = (size_t)std::stol(cell_value);
	  this->flags |= F_START;
	}
  else
	handle_fatal_error("In the format of the sheet while reading ticker");
}

void excel_parser::set_file_path(const std::string file_path)
{
  this->file_path = file_path;
}

void excel_parser::clear_flags()
{
  this->flags = 0x0;
}

void excel_parser::close_book()
{
  xlsxioread_close(this->book);
}
void excel_parser::clear_all()
{
  this->file_path = "";
  xlsxioread_close(this->book);
  std::vector<std::string>().swap(this->sheet_names);
  this->ticker_index = 0;
  this->flags = 0x0;
}

bool excel_parser::jump_rows(const xlsxioreadersheet sheet, const size_t cuant)
{
  for (size_t i = 0; i < cuant; i++)
	{
	  if (!xlsxioread_sheet_next_row(sheet))
		return(false);
	}
  return(true);
}
