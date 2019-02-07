// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser_daily.cpp                                                         //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/02/07 11:22:40 by Mateo                                    //
//   Updated: 2019/02/07 14:57:57 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <stdlib.h>

// constructor
excel_parser_daily::excel_parser_daily() : client(), data_base()
{
  this->bloom_tickers = NULL;
	// this->ticker_retries = NULL;
	this->n_bloom_tickers = 0;
  this->file_path = "";
  this->flags = 0x0;
  this->vm_id = 0;
	// this->queue = {0, 0, NULL};
}

// print fatal error and exit
void excel_parser_daily::handle_fatal_error(const std::string message)
{
  std::cerr << "Fatal error: " << message << "\n";
  exit(EXIT_FAILURE);
}

// check if a string is all digits
bool excel_parser_daily::issdigit(char *str)
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

// load in memory the tickers and connect to the data base
void excel_parser_daily::init()
{
	client::init();
	this->bloom_tickers = client::get_tickers(&n_bloom_tickers);
	printf("Got %d tickers\n", n_bloom_tickers);
	// ticker_retries = new int[this->n_bloom_tickers]();
	// this->tickers_in_queue = new bool[this->n_bloom_tickers];
	data_base::connect_db(DB_NAME, DB_USER, DB_PASS, DB_HOST);
}

// load the book from the path
void excel_parser_daily::load_book(const std::string file_path)
{
	unsigned char i;
	std::string sheet_arr[SHEET_NB_DAILY] = SHEET_ARR_DAILY;

  this->file_path = file_path;
	// open the book
  if ((book = xlsxioread_open(file_path.c_str())) == NULL)
		{
			std::cerr << "Error opening: " << file_path << "\n";
			exit(EXIT_FAILURE);
		}
  //list available sheets
  xlsxioreadersheetlist sheetlist;
  const XLSXIOCHAR* sheetname;
	// get the sheets from the book and compare with the expected ones (to avoid hiddwen sheets or unwanted)
  if ((sheetlist = xlsxioread_sheetlist_open(book)) != NULL)
		{
			i = 0;
			// for each sheet in the book compare with the ones expected in header
			while ((sheetname = xlsxioread_sheetlist_next(sheetlist)) != NULL)
				{
					for (int j = 0; j < SHEET_NB_DAILY; j++)
						{
							if (sheetname == sheet_arr[j])
								{
									this->sheet_names[i++] = sheetname;
									break;
								}
						}
				}
			xlsxioread_sheetlist_close(sheetlist);
			if (i != SHEET_NB_DAILY)
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

// check the specified marks in the sheets to navigate the seet in memory easyer
void excel_parser_daily::check_marks(sheet_t *sheet, std::string cell_value, size_t i, size_t j)
{
	//if a ticker starts restart error in ticker is there was in the previous
	// ticker and add the reference to the start of the ticker
	if (cell_value == TICKER_START)
		{
			sheet->flags &= ~(FS_ERROR_IN_TICKER);
			sheet->ticker_id.push_back({i, j});
		}
}

// handle an error in the cell
bool excel_parser_daily::mark_cell_error(std::string cell_value, sheet_t *sheet)
{
	std::string error_arr[] = ERROR_ARR;
	size_t i = 0;

	// check the cell value compare with the errors written in the header
	while (!error_arr[i].empty())
		{
			// if an error is find the ticker is not considered by removing the references
			// and setting the flaf to error in ticker
			if (error_arr[i] == cell_value)
				{
					if (sheet->end_tick.size() < sheet->ticker_id.size())
						sheet->ticker_id.pop_back();
					else if (sheet->end_tick.size() > sheet->ticker_id.size())
						sheet->end_tick.pop_back();
					sheet->flags |= FS_ERROR_IN_TICKER;
					return (true);
				}
			i++;
		}
	return (false);
}

// load the whole book in memory
void excel_parser_daily::parse_book()
{
  xlsxioreadersheet sheet;
	char *cell_value;
	size_t row_i, cell_j;

	memset(sheets, 0, sizeof(struct sheet_s) * SHEET_NB_DAILY);
	// std::fill(tickers_in_queue, tickers_in_queue + n_bloom_tickers, false);
	// for each sheet
  for (int i = 0; i < SHEET_NB_DAILY; i++)
		{
			//open the sheet
			if ((sheet = xlsxioread_sheet_open(book, sheet_names[i].c_str(),
																				 XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL)
				{
					row_i = 0;
					// while the sheet is not empty and the batch incomplet
					while (!(sheets[i].flags & FS_END_OF_SHEET) && xlsxioread_sheet_next_row(sheet))
						{
							cell_j = 0;
							// if the previous row was empty, the memory is reutulized
							if (!(sheets[i].flags & FS_EMPTY_ROW))
								sheets[i].sheet.push_back(std::vector<std::string>());
							// for each cell in the row
							while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL)
								{
									// clean empty row flag
									sheets[i].flags &= ~(FS_EMPTY_ROW);
									// check for errors in cell
									mark_cell_error(cell_value, sheets + i);
									// if cell is the first check for marks to add references
									if (cell_j == 0)
										check_marks(sheets + i, cell_value, row_i, cell_j);
									// if the row is empty or there is an error clean the row and mark as empty to reutilize
									if ((*cell_value == 0 && cell_j == 0) || sheets[i].flags & FS_ERROR_IN_TICKER)
										{
											free(cell_value);
											sheets[i].sheet[row_i].clear();
											sheets[i].flags |= FS_EMPTY_ROW;
											sheets[i].flags |= FS_END_OF_SHEET;
											break;
										}
									// if the cell is empty the row has finished
									else if (*cell_value == 0)
										{
											free(cell_value);
											break;
										}
									// charge the cell in memory
									sheets[i].sheet[row_i].push_back(cell_value);
									cell_j++;
									free(cell_value);
								}
							// increase row if row was not empty
							if (!(sheets[i].flags & FS_EMPTY_ROW))
								row_i++;
						}
				}
			//close sheet
			xlsxioread_sheet_close(sheet);
		}
	//close book
	xlsxioread_close(book);
	flags = 0x0;
	for (int i = 0; i < SHEET_NB_DAILY; i++)
		sheets[i].flags = 0x0;
	// parse each ticker in book
	parse_tickers();
}

void excel_parser_daily::parse_tickers()
{
	ticker_daily_t tick;
	size_t i, j;
	std::string values("");

	// loop until all tickers are done
	while (!(flags & F_END_PARSING))
		{
			// for each sheet
			for (int k = 0; k < SHEET_NB_DAILY; k++)
				{
					//check if sheet finished
					if (sheets[k].flags & FS_END_OF_SHEET)
						break;
					//load the start of the ticker from the reference
					i = sheets[k].ticker_id[sheets[k].ticker_id_iter].i;
					j = sheets[k].ticker_id[sheets[k].ticker_id_iter].j;
					// if the sheet is the first load the info, like the period date
					if (k == 0)
						{
							//name
							tick.ticker_capiq = sheets[k].sheet[i][j + 1];
							//index
							tick.ticker_index = std::stol(sheets[k].sheet[i + 1][j]);
							tick.ticker_bbg = bloom_tickers[tick.ticker_index];
						}
					// the info starts 4 rows away from the start
					i += 4;
					for (; i < sheets[k].sheet.size(); i++)
						{
							values += "(\'" + tick.ticker_bbg + "\', \'" + tick.ticker_capiq + "\'";
							for(j = 0; j < sheets[k].sheet[i].size(); j++)
								{
									if (j)
										values += ", " + sheets[k].sheet[i][j];
									else
										values += ", \'" + parse_excel_date(sheets[k].sheet[i][j]) + "\'";
								}
							if (i != (sheets[k].sheet.size() - 1))
								values += "),\n";
							else
								values += ");";
						}
					if (k == SHEET_NB_DAILY - 1)
						flags |= F_END_PARSING;
				}
			// upload the batch of tickers
			data_base::upload_ticker_daily(&values);
		}
}

// from excel format to SQL format
std::string excel_parser_daily::parse_excel_date(const std::string date_str)
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

//cleaning functions
void excel_parser_daily::clear_flags()
{
  this->flags = 0x0;
}

void excel_parser_daily::close_book()
{
  xlsxioread_close(this->book);
}

void excel_parser_daily::clear_all()
{
  this->file_path = "";
  this->flags = 0x0;
	for (int i = 0; i < SHEET_NB_DAILY; i++)
		{
			sheets[i].flags = 0x0;
			sheets[i].ticker_id.clear();
			sheets[i].end_tick.clear();
			sheets[i].fil_date.clear();
			sheets[i].ticker_id_iter = 0;
			sheets[i].end_ticker_iter = 0;
			sheets[i].fil_date_iter = 0;
			for (size_t j = 0; j < sheets[i].sheet.size(); j++)
				sheets[i].sheet[j].clear();
			sheets[i].sheet.clear();
		}
}

void excel_parser_daily::free_all()
{
  this->file_path = "";
  this->flags = 0x0;
	for (int i = 0; i < SHEET_NB_DAILY; i++)
		{
			sheets[i].flags = 0x0;
			std::vector<coord_t>().swap(sheets[i].ticker_id);
			std::vector<coord_t>().swap(sheets[i].end_tick);
			std::vector<coord_t>().swap(sheets[i].fil_date);
			sheets[i].ticker_id_iter = 0;
			sheets[i].end_ticker_iter = 0;
			sheets[i].fil_date_iter = 0;
			size_t six = sheets[i].sheet.size();
			for (size_t j = 0; j < six; j++)
				std::vector<std::string>().swap(sheets[i].sheet[j]);
			std::vector<std::vector<std::string>>().swap(sheets[i].sheet);
		}
}

void excel_parser_daily::clear_bloom_tickers()
{
	for (unsigned short i = 0; i < n_bloom_tickers; i++)
		delete[] bloom_tickers[i];
	delete[] bloom_tickers;
}

// void excel_parser_daily::clear_ticker_retries()
// {
// 	delete[] ticker_retries;
// }
