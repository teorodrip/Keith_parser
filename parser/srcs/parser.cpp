// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.cpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/10 17:57:13 by Mateo                                    //
//   Updated: 2019/01/22 18:19:05 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <stdlib.h>

excel_parser::excel_parser() : client(), data_base()
{
  this->bloom_tickers = NULL;
	this->ticker_retries = NULL;
	this->n_bloom_tickers = 0;
  this->file_path = "";
	this->ticker_name = "";
	this->period_dates = NULL;
	this->fil_date = "";
  this->flags = 0x0;
  this->ticker_index = 0;
  this->vm_id = 0;
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

void excel_parser::init()
{
	client::init();
	this->bloom_tickers = client::get_tickers(&n_bloom_tickers);
	printf("Got %d tickers\n", n_bloom_tickers);
	ticker_retries = new int[this->n_bloom_tickers]();
	this->period_dates = new std::string[this->n_bloom_tickers];
	data_base::connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
}

void excel_parser::load_book(const std::string file_path)
{
	unsigned char i;
	std::string sheet_arr[SHEET_NB] = SHEET_ARR;

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
			i = 0;
			while ((sheetname = xlsxioread_sheetlist_next(sheetlist)) != NULL)
				{
					for (int j = 0; j < SHEET_NB; j++)
						{
							if (sheetname == sheet_arr[j])
								{
									this->sheet_names[i++] = sheetname;
									break;
								}
						}
				}
			xlsxioread_sheetlist_close(sheetlist);
			if (i != SHEET_NB)
				{
					std::cerr << "Error: wrong number of sheets!\n";
					exit(EXIT_FAILURE);
				}
		}
  else
		{
			std::cerr << "Error: listing the sheets\n";
			exit(EXIT_FAILURE);
		}
}

void excel_parser::parse_book()
{
  xlsxioreadersheet sheet;
  ticker_json_t j_quarter;//before half ticker
  ticker_json_t j_year;//after half ticker

	this->current_sheet = 0;
  for (std::string sheet_name : sheet_names)
		{
			if ((sheet = xlsxioread_sheet_open(book, sheet_name.c_str(),
																				 XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL)
				{
					j_quarter = {0, NULL};
					j_year = {0, NULL};
					this->flags = 0x0;
					while (!(this->flags & F_END_PARSING) && xlsxioread_sheet_next_row(sheet))
						{
							parse_row(sheet, &j_quarter, &j_year);
							if (this->flags & F_FATAL_ERROR)
								break;
						}
					//if set flags to 0 there is segfault in launcher must check
					xlsxioread_sheet_close(sheet);
				}
			delete[] j_quarter.j;
			delete[] j_year.j;
			j_quarter.j = NULL;
			j_year.j = NULL;
			this->current_sheet++;
			if (this->flags & F_FATAL_ERROR)
				break;
		}
}

void excel_parser::parse_row(const xlsxioreadersheet sheet, ticker_json_t *j_quarter,
														 ticker_json_t *j_year)
{
  ticker_json_t *j;
  size_t cell_j;
  char *cell_value;
  std::string current_field;

  cell_j = 0;
  j = (this->flags & F_HALF) ? j_year : j_quarter;
  while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL)
		{
			if (!(this->flags & F_ERROR_IN_TICKER) && handle_cell_error(cell_value))
				{
					mark_cell_error(cell_value);
					break;
				}
			if (cell_j == 0)
				{
					if (!strcmp(cell_value, END_TICKER))
						{
							if (!(this->flags & F_ERROR_IN_TICKER) &&
									!this->period_dates[this->ticker_index].empty() &&
									((this->current_sheet && !this->fil_date.empty()) || !this->current_sheet))
								{
									if (data_base::upload_ticker_period(ticker_name,
																											this->period_dates[this->ticker_index],
																											bloom_tickers[ticker_index],
																											j->j->dump(),
																											this->fil_date,
																											this->current_sheet))
										{
											this->flags |= F_FATAL_ERROR;
											break;
										}
								}
							else
								std::cerr << "Some error in ticker " + ticker_name + "\n";
							// not add the flag but set flags to end so the others are cleared
							this->flags = F_END;
							jump_rows(sheet, 1); //jump empty row after end
							//upload to data base
							delete[] j_quarter->j;
							delete[] j_year->j;
							j_quarter->j = NULL;
							j_year->j = NULL;
							break;
						}
					else if (this->flags & F_ERROR_IN_TICKER)
						break;
					else if (!strcmp(cell_value, TICKER_START))
						{
							if (flags & F_START)
								{
									mark_cell_error(cell_value);
									break;
								}
							this->flags = F_START;
							init_ticker(sheet);
							jump_rows(sheet, 1); //next row contains index
							init_index(sheet);
							jump_rows(sheet, 2); //jump company name and go to dates rows
							init_date(sheet, j);
							break; //continue parsing for next row
						}
					else if (!strcmp(cell_value, HALF_TICKER))
						{
							if (!(flags & F_START) || (flags & F_HALF))
								{
									mark_cell_error(cell_value);
									break;
								}
							this->flags |= F_HALF;
							jump_rows(sheet, 3); //go to dates row
							j = j_year; //reached half of ticker
							init_date(sheet, j);
							break; //continue for next row
						}
					else if (*cell_value == '\0')
						{
							if (this->flags & F_END)
								this->flags = F_END_PARSING;
							break; // if not ended parsing jump row
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
				{
					(j->j[cell_j - 1])[current_field] = cell_value;
					if (this->flags & F_FIL_DATE)
						{
							this->fil_date = parse_excel_date(std::stoi(cell_value));
							this->flags &= ~(F_FIL_DATE);
						}
				}
			cell_j++;
			free(cell_value);
		}
	free(cell_value);
}

void excel_parser::mark_cell_error(std::string cell_value)
{
	std::cerr << "Find error relative to: " + cell_value + "\n";
	this->flags |= F_ERROR_IN_TICKER;
}

bool excel_parser::handle_cell_error(std::string value)
{
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
					return (true);
				}
			i++;
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
					return (true);
				}
			i++;
		}
	return (false);
}

std::string excel_parser::parse_excel_date(int date)
{
	int n_day, n_month, n_year;
	std::string parsed_date;
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

//this will create the array of json for each date of the ticker
void excel_parser::init_date(const xlsxioreadersheet sheet, ticker_json_t *j)
{
  char *cell_value;

  j->len = (size_t)-1; //the first cell does not count
  while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL && *cell_value != '\0')
		{
			if (!this->current_sheet && !j->len)
				this->period_dates[this->ticker_index] = parse_excel_date(std::stoi(cell_value));
			j->len++;
			free(cell_value);
		}
	free(cell_value);
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
	free(cell_value);
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
	free(cell_value);
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

void excel_parser::clear_bloom_tickers()
{
	for (unsigned short i = 0; i < n_bloom_tickers; i++)
		delete[] bloom_tickers[i];
	delete[] bloom_tickers;
}

void excel_parser::clear_ticker_retries()
{
	delete[] ticker_retries;
}
