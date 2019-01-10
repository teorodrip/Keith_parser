// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.cpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/10 17:57:13 by Mateo                                    //
//   Updated: 2019/01/10 19:18:45 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"

excel_parser::excel_parser()
{
}

excel_parser::excel_parser(std::string file_path)
{
	this->file_path = file_path;
}

void excel_parser::init()
{
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
				{
					sheet_names.push_back(sheetname);
				}
			xlsxioread_sheetlist_close(sheetlist);
		}
	else
		{
			std::cerr << "Error: listing the sheets\n";
			exit(EXIT_FAILURE);
		}
	for (std::string str : sheet_names)
		std::cout << str << "\n";
}
