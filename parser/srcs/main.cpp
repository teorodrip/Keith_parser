// ************************************************************************** //
//                                                                            //
//                                                                            //
//   main.cpp                                                                 //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/04 17:51:41 by Mateo                                    //
//   Updated: 2019/01/09 18:45:54 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"

int main()
{
	client cli= client();
	size_t n_tickers;
	unsigned char n_vm;

	cli.init();
	n_tickers = cli.get_number_tickers();
	printf("Get %lu tickers from launcher\n", n_tickers);
	n_vm = cli.get_watching_directories();
	printf("Get %u tickers from launcher\n", n_vm);
	dir_watcher watcher = dir_watcher(0);
	while(true)
		{
			watcher.watch_directory();
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
