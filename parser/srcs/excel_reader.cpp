// ************************************************************************** //
//                                                                            //
//                                                                            //
//   excel_reader.cpp                                                         //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/25 11:31:34 by Mateo                                    //
//   Updated: 2019/01/25 17:35:22 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/excel_reader.hpp"

excel_reader::excel_reader()
{

}

bool excel_reader::load_book(const std::string file_path,
														 const std::regex *sh_exceptions,
														 const size_t regex_siz)
{
	xlsxioreadersheetlist sheet_list;
	char *sheet_name;
	bool regex_matched;

	if ((book = xlsxioread_open(file_path.c_str())) == NULL)
		{
			fprintf(stderr, F_ERROR("Opening %s"), file_path.c_str());
			return(false);
		}
	if ((sheet_list = xlsxioread_sheetlist_open(book)) == NULL)
		{
			fprintf(stderr, F_ERROR("Listing book sheets"));
			xlsxioread_close(book);
			return(false);
		}
	while ((sheet_name = (char *)xlsxioread_sheetlist_next(sheet_list)) != NULL)
		{
			regex_matched = false;
			for (size_t i = 0; i < regex_siz; i++)
				{
					if (std::regex_match(sheet_name, sh_exceptions[i]))
						{
							regex_matched = true;
							break;
						}
				}
			if (!regex_matched)
				sheet_names.push_back(sheet_name);
			free(sheet_name);
		}
	xlsxioread_sheetlist_close(sheet_list);
	return (true);
}

bool excel_reader::parse_book(error_handler *err)
{
	xlsxioreadersheet *sheets;
	unsigned char parser_status;
	ticker_parser p;
	//open sheets
	sheets = new xlsxioreadersheet[sheet_names.size()];
	for (size_t i = 0; i != sheet_names.size(); i++)
		{
			if ((sheets[i] = xlsxioread_sheet_open(book, sheet_names[i].c_str(),
																						 XLSXIOREAD_SKIP_EMPTY_ROWS)) == NULL)
				{
					fprintf(stderr, F_ERROR("Opening sheet %s"), sheet_names[i].c_str());
					delete[] sheets;
					return (false);
				}
		}
	//parse each ticker of the sheet
	do
		{
			//parsing function
			parser_status = p.parse_ticker(sheets, sheet_names.size());
			if (parser_status == F_FATAL_ERROR)
				{
					fprintf(stderr, F_ERROR("While parsing tickers"));
					delete[] sheets;
					return (false);
				}
		}
	while ((parser_status != F_END_PARSING));
	delete[] sheets;
	return (true);
}
