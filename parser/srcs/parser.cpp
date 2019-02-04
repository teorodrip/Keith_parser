// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.cpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/10 17:57:13 by Mateo                                    //
//   Updated: 2019/02/04 15:25:10 by Mateo                                    //
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
  this->flags = 0x0;
  this->vm_id = 0;
	this->queue = {0, 0, NULL};
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
	this->tickers_in_queue = new bool[this->n_bloom_tickers];
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

void excel_parser::check_marks(sheet_t *sheet, std::string cell_value, size_t i, size_t j)
{
	if (cell_value == TICKER_START)
		{
			sheet->flags &= ~(FS_ERROR_IN_TICKER);
			sheet->ticker_id.push_back({i, j});
		}
	else if (cell_value == END_TICKER)
		{
			if (sheet->end_tick.size() == BATCH_SIZE)
				sheet->flags |= FS_END_OF_SHEET;
			if (!(sheet->flags & FS_ERROR_IN_TICKER))
				sheet->end_tick.push_back({i, j});
		}
	else if (cell_value == FIL_DATE && !(sheet->flags & FS_ERROR_IN_TICKER))
		sheet->fil_date.push_back({i, j});
}

bool excel_parser::mark_cell_error(std::string cell_value, sheet_t *sheet)
{
	std::string error_arr[] = ERROR_ARR;
	size_t i = 0;

	while (!error_arr[i].empty())
		{
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

void excel_parser::parse_book()
{
  xlsxioreadersheet sheet;
	char *cell_value;
	size_t row_i, cell_j;

	memset(sheets, 0, sizeof(struct sheet_s) * SHEET_NB);
	std::fill(tickers_in_queue, tickers_in_queue + n_bloom_tickers, false);
  for (int i = 0; i < SHEET_NB; i++)
		{
			if ((sheet = xlsxioread_sheet_open(book, sheet_names[i].c_str(),
																				 XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL)
				{
					row_i = 0;
					while (!(sheets[i].flags & FS_END_OF_SHEET) && xlsxioread_sheet_next_row(sheet))
						{
							cell_j = 0;
							if (!(sheets[i].flags & FS_EMPTY_ROW))
								sheets[i].sheet.push_back(std::vector<std::string>());
							while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL)
								{
									sheets[i].flags &= ~(FS_EMPTY_ROW);
									mark_cell_error(cell_value, sheets + i);
									if (cell_j == 0)
										check_marks(sheets + i, cell_value, row_i, cell_j);
									if ((*cell_value == 0 && cell_j == 0) || sheets[i].flags & FS_ERROR_IN_TICKER)
										{
											free(cell_value);
											sheets[i].sheet[row_i].clear();
											sheets[i].flags |= FS_EMPTY_ROW;
											break;
										}
									else if (*cell_value == 0)
										{
											free(cell_value);
											break;
										}
									sheets[i].sheet[row_i].push_back(cell_value);
									cell_j++;
									free(cell_value);
								}
							if (!(sheets[i].flags & FS_EMPTY_ROW))
								row_i++;
						}
				}
			xlsxioread_sheet_close(sheet);
		}
	xlsxioread_close(book);
	flags = 0x0;
	for (int i = 0; i < SHEET_NB; i++)
		sheets[i].flags = 0x0;
	parse_tickers();
}

void excel_parser::parse_tickers()
{
	ticker_json_t tick;
	size_t i, j, row_siz;

	while (!(flags & F_END_PARSING))
		{
			for (int k = 0; k < SHEET_NB; k++)
				{
					//check if sheet finished
					if (sheets[k].flags & FS_END_OF_SHEET)
						break;
					//load ticker
					i = sheets[k].ticker_id[sheets[k].ticker_id_iter].i;
					j = sheets[k].ticker_id[sheets[k].ticker_id_iter].j;
					if (k == 0)
						{
							//name
							tick.ticker_capiq = sheets[k].sheet[i][j + 1];
							//index
							tick.ticker_index = std::stol(sheets[k].sheet[i + 1][j]);
							//dates year
							tick.dates_year = &(sheets[k].sheet[i + 3]);
						}
					i += 5;
					j = 0;
					tick.j_year[k].resize(tick.dates_year->size());
					while (!(sheets[k].sheet[i].size()) || sheets[k].sheet[i][j] != HALF_TICKER)
						{
							if (sheets[k].sheet[i].size())
								{
									row_siz = sheets[k].sheet[i].size() < tick.dates_year->size() ? sheets[k].sheet[i].size() : tick.dates_year->size();
									for (size_t p = 1; p < row_siz; p++)
										{
											try
												{
													((tick.j_year[k])[p - 1])[sheets[k].sheet[i][0]] = sheets[k].sheet[i][p];
												}
											catch (const std::exception &e)
												{
													printf("Error: %s\nIn: %s\n", e.what(), sheets[k].sheet[i][p].c_str());
													exit(2);
												}
										}
								}
							i++;
						}
					if (k == 0)
						{
							//dates quarter
							tick.dates_quarter = &(sheets[k].sheet[i + 2]);
						}
					i += 4;
					tick.j_quarter[k].resize(tick.dates_quarter->size());
					while (!(sheets[k].sheet[i].size()) || sheets[k].sheet[i][j] != END_TICKER)
						{
							if (sheets[k].sheet[i].size())
								{
									row_siz = sheets[k].sheet[i].size() < tick.dates_quarter->size() ? sheets[k].sheet[i].size() : tick.dates_quarter->size();
									for (size_t p = 1; p < row_siz; p++)
										{
											try
												{
													((tick.j_quarter[k])[p - 1])[sheets[k].sheet[i][0]] = sheets[k].sheet[i][p];
												}
											catch (const std::exception &e)
												{
													printf("Error: %s\nIn: %s\n", e.what(), sheets[k].sheet[i][p].c_str());
													exit(2);
												}
										}
								}
							i++;
						}
					sheets[k].end_ticker_iter++;
					sheets[k].ticker_id_iter++;
					sheets[k].fil_date_iter += 2;
					if (sheets[k].end_ticker_iter >= sheets[k].end_tick.size())
						sheets[k].flags |= FS_END_OF_SHEET;
					if ((k == SHEET_NB - 1) && (sheets[k].flags & FS_END_OF_SHEET))
						flags |= F_END_PARSING;
				}
			data_base::upload_ticker(&tick, bloom_tickers[tick.ticker_index], sheets);
		}
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
  this->flags = 0x0;
	for (int i = 0; i < SHEET_NB; i++)
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

void excel_parser::free_all()
{
  this->file_path = "";
  this->flags = 0x0;
	for (int i = 0; i < SHEET_NB; i++)
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
