// ************************************************************************** //
//                                                                            //
//                                                                            //
//   main.cpp                                                                 //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/04 17:51:41 by Mateo                                    //
//   Updated: 2019/01/31 17:32:25 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <dirent.h>

static void parse_from_dir(std::string path)
{
	DIR *dir;
	struct dirent *ent;
	excel_parser parser;
	std::string file_string;
	std::string path_to_file;
	// std::regex file_format (".*\\.xlsx");
	std::regex file_format ("^(([^~])+).*\\.xlsx");

	if ((dir = opendir(path.c_str())) == NULL)
		{
			printf("Error: opening dir %s\n", path.c_str());
			return;
		}
	parser.init();
	while ((ent = readdir(dir)) != NULL)
		{
			file_string = ent->d_name;
			if (!std::regex_match(file_string.begin(), file_string.end(), file_format))
				continue;
			parser.client::signal_parsing();
			printf("Parsing %s\n", ent->d_name);
			path_to_file = path + file_string;
			parser.load_book(path_to_file);
			parser.parse_book();
			parser.client::signal_end_parsing();
			printf("Done\n");
			parser.clear_all();
			// goto label;
		}
	// label:
	closedir(dir);
	parser.clear_bloom_tickers();
	parser.clear_ticker_retries();
	parser.data_base::finish_db();
}

static void parse_from_watcher()
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
							printf("Done\n");
							parser.clear_all();
							// goto label;
						}
				}
			usleep(9000000);
		}
	while (parser.client::check_end());
	// label:
	parser.clear_bloom_tickers();
	parser.clear_ticker_retries();
	parser.data_base::finish_db();
	delete[] watcher;
}

int main(int argc, char **argv)
{
	if (argc == 1)
		parse_from_watcher();
	else if (argc == 2)
		parse_from_dir(argv[1]);
	else
		printf("Nothing to do\n");
	return (1);
}
