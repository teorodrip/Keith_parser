// ************************************************************************** //
//                                                                            //
//                                                                            //
//   main.cpp                                                                 //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/04 17:51:41 by Mateo                                    //
//   Updated: 2019/01/16 19:21:22 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"

int main()
{
	client cli= client();
	unsigned short n_tickers;
	char **bloom_tick;
	// unsigned char n_vm;
	// dir_watcher *watcher;

	cli.init();
	bloom_tick = cli.get_tickers(&n_tickers);
	for (int i = 0; i < n_tickers; i++)
		std::cout << bloom_tick[i] << "\n";
	(void)ticker_retries;
	// printf("Get %lu tickers from launcher\n", n_tickers);
	// n_vm = cli.get_watching_directories();
	// printf("Get %u tickers from launcher\n", n_vm);
	// watcher = new dir_watcher[n_vm];
	// for (int i = 0; i < n_vm; i++)
	// 	watcher[i] = dir_watcher(i);
	// while(true)
	// 	{
	// 		for (int i = 0; i < n_vm; i++)
	// 			watcher[i].watch_directory();
	// 		usleep(100000);
	// 	}

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
