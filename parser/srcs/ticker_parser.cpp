// ************************************************************************** //
//                                                                            //
//                                                                            //
//   ticker_parser.cpp                                                        //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/25 14:37:34 by Mateo                                    //
//   Updated: 2019/01/25 18:53:02 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/ticker_parser.hpp"

ticker_parser::ticker_parser()
{
	flags = 0x0;
	err = NULL;
}

ticker_parser::ticker_parser(error_handler *err)
{
	flags = 0x0;
	this->err = err;
}

unsigned char ticker_parser::parse_ticker(const xlsxioreadersheet *sheets, const size_t len)
{
	char *cell_value;
	size_t cell_j;

	for (size_t i = 0; i < len; i++)
		{
			do
				{
					if (!xlsxioread_sheet_next_row(sheets[i]))
						return (F_END_PARSING);
					cell_j = 0;
					while ((cell_value = xlsxioread_sheet_next_cell(sheets[i])) != NULL)
						{
							if ((flags & F_START) &&
									(err->check_cell(cell_value, ticker_index)) != E_NO_ERRORS)
								flags |= F_ERROR_IN_TICKER;
							if (cell_j == 0)
								{
									if (!strcmp(cell_value, END_TICKER))
										{
											//TODO continue here
										}
								}
						}
				}
			while (!(flags & F_END_TICKER) && !(flags & F_END_PARSING));
		}
}
