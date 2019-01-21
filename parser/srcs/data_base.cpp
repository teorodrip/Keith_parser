// ************************************************************************** //
//                                                                            //
//                                                                            //
//   data_base.cpp                                                            //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/18 15:27:33 by Mateo                                    //
//   Updated: 2019/01/21 11:10:39 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <fstream>

data_base::data_base()
{

}

void data_base::upload_data(ticker_json_t data)
{
	std::string path = "/home/unchartech_2/json_out/output.json";
	std::ofstream f;

	f.open(path, std::ofstream::app);
	for (size_t i = 0; i < data.len; i++)
		f << data.j[i].dump();
	f.close();
}
