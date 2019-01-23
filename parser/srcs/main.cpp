// ************************************************************************** //
//                                                                            //
//                                                                            //
//   main.cpp                                                                 //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/04 17:51:41 by Mateo                                    //
//   Updated: 2019/01/23 18:09:27 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"

int main()
{
	excel_parser parser;
	unsigned char n_vm;
	dir_watcher *watcher;
	char file_to_parse[NAME_MAX];
	std::string file_string;
	std::string path_to_file;
	// std::regex file_format (".*\\.xlsx");
	std::regex file_format ("^(([^~])+).*\\.xlsx");

	parser.init();
	n_vm = parser.client::get_watching_directories() + 1;
	printf("Watching %d virtual machines\n", n_vm);
	watcher = new dir_watcher[n_vm];
	for (int i = 0; i < n_vm; i++)
		watcher[i] = dir_watcher(DEFAULT_PATH + std::to_string(i) + "/");
	do
		{
			for (int i = 0; i < n_vm; i++)
				{
					while (watcher[i].watch_directory(file_to_parse))
						{
							file_string = file_to_parse;
							if (!std::regex_match(file_string.begin(), file_string.end(), file_format))
								continue;
							parser.client::signal_parsing();
							printf("Parsing %s\n", file_to_parse);
							path_to_file = DEFAULT_PATH + std::to_string(i) + "/" + file_to_parse;
							parser.load_book(path_to_file);
							parser.parse_book();
							parser.client::signal_end_parsing();
							parser.clear_all();
						}
				}
			usleep(9000000);
		}
	while (parser.client::check_end());
	parser.clear_bloom_tickers();
	parser.clear_ticker_retries();
	parser.clear_period_dates();
	parser.data_base::finish_db();
	delete[] watcher;

	// xlsxioreader book;
	// xlsxioreadersheet sheet;
	// char *cell_value;
	// char *sheet_name = NULL;

	// if ((book = xlsxioread_open(FILE_NAME)) == NULL)
	// 	{
	// 		std::cerr << "Error opening: " << FILE_NAME << "\n";
	// 		exit(2);
	// 	}
	// if ((sheet = xlsxioread_sheet_open(book, sheet_name, XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL)
	// 	{
	// 		while (xlsxioread_sheet_next_row(sheet))
	// 			{
	// 				while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL)
	// 					{
	// 						std::cout << cell_value << "\t";
	// 						free(cell_value);
	// 					}
	// 				std::cout << "\n";
	// 			}
	// 		xlsxioread_sheet_close(sheet);
	// 	}
	// xlsxioread_close(book);
	// return (0);
}
