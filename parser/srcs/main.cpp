// ************************************************************************** //
//                                                                            //
//                                                                            //
//   main.cpp                                                                 //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/04 17:51:41 by Mateo                                    //
//   Updated: 2019/01/04 18:27:00 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include <xlsxio_read.h>
#include <iostream>

#define FILE_NAME "./sheet.xlsx"

int main()
{
	xlsxioreader book;
	xlsxioreadersheet sheet;
	char *cell_value;
	char *sheet_name = NULL;

	if ((book = xlsxioread_open(FILE_NAME)) == NULL)
		{
			std::cerr << "Error opening: " << FILE_NAME << "\n";
			exit(2);
		}
	if ((sheet = xlsxioread_sheet_open(book, sheet_name, XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL)
		{
			while (xlsxioread_sheet_next_row(sheet))
				{
					while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL)
						{
							std::cout << cell_value << "\t";
							free(cell_value);
						}
					std::cout << "\n";
				}
			xlsxioread_sheet_close(sheet);
		}
	xlsxioread_close(book);
	return (0);
}
