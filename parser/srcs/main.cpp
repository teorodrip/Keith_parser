// ************************************************************************** //
//                                                                            //
//                                                                            //
//   main.cpp                                                                 //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/04 17:51:41 by Mateo                                    //
//   Updated: 2019/01/22 18:36:04 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"

int main()
{
	excel_parser parser;
	unsigned char n_vm;
	dir_watcher *watcher;
	char file_to_parse[NAME_MAX];
	std::string path_to_file;

	parser.init();
	n_vm = parser.client::get_watching_directories();
	printf("Watching %d virtual machines\n", n_vm);
	watcher = new dir_watcher[n_vm];
	for (int i = 0; i < n_vm; i++)
		watcher[i] = dir_watcher(DEFAULT_PATH + std::to_string(i) + "/");
	while(true)
		{
			for (int i = 0; i < n_vm; i++)
				{
					while (watcher[i].watch_directory(file_to_parse))
						{
							printf("Parsing %s\n", file_to_parse);
							path_to_file = DEFAULT_PATH + std::to_string(i) + "/" + file_to_parse;
							parser.load_book(path_to_file);
							parser.parse_book();
							parser.clear_bloom_tickers();
							parser.clear_ticker_retries();
							parser.clear_period_dates();
							parser.data_base::finish_db();
							delete[] watcher;
							parser.clear_all();
							return 1;
						}
				}
			usleep(100000);
		}

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
