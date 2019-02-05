// ************************************************************************** //
//                                                                            //
//                                                                            //
//   main.cpp                                                                 //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/04 17:51:41 by Mateo                                    //
//   Updated: 2019/02/05 10:48:03 by Mateo                                    //
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
	std::regex file_format ("^(([^~])+).*\\.xlsx");

	// open the dirctory
	if ((dir = opendir(path.c_str())) == NULL)
		{
			printf("Error: opening dir %s\n", path.c_str());
			return;
		}
	parser.init();
	// loop through all the files inside
	while ((ent = readdir(dir)) != NULL)
		{
			file_string = ent->d_name;
			// avoid wrong files
			if (!std::regex_match(file_string.begin(), file_string.end(), file_format))
				continue;
			//same as below
			parser.client::signal_parsing();
			printf("Parsing %s\n", ent->d_name);
			path_to_file = path + file_string;
			parser.load_book(path_to_file);
			parser.parse_book();
			parser.client::signal_end_parsing();
			printf("Done\n");
			parser.clear_all();
		}
	// end proper
	closedir(dir);
	parser.free_all();
	parser.clear_bloom_tickers();
	// parser.clear_ticker_retries();
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
	std::regex file_format ("^(([^~])+).*\\.xlsx");

	parser.init();
	// get how many vm we need to see
	n_vm = parser.client::get_watching_directories() + 1;
	printf("Watching %d virtual machines\n", n_vm);
	watcher = new dir_watcher[n_vm];
	// create all the watcher objects to watch each independent directory (this is going to change)
	for (int i = 0; i < n_vm; i++)
		watcher[i] = dir_watcher(DEFAULT_PATH + std::to_string(i) + "/");
	// parser will ask each loop if the program has end, the conditions are:
	// no tickers in list,
	// no tickers in queue (in this version there are no queue because is not needed, will implement that in the modular version)
	// no vm is running
	// the parse is also not running
	do
		{
			for (int i = 0; i < n_vm; i++)
				{
					// watch for changes in directory
					while (watcher[i].watch_directory(file_to_parse))
						{
							file_string = file_to_parse;
							// check if file fits the format (avoid tmp files and wrong ones)
							if (!std::regex_match(file_string.begin(), file_string.end(), file_format))
								continue;
							// send the signal of parsing
							parser.client::signal_parsing();
							printf("Parsing %s\n", file_to_parse);
							path_to_file = DEFAULT_PATH + std::to_string(i) + "/" + file_to_parse;
							// load the book from the path
							parser.load_book(path_to_file);
							parser.parse_book();
							parser.client::signal_end_parsing();
							printf("Done\n");
							// clear all for next use (not free) so the vector will allocate
							// some memory in the first instance and reutilized the next ones
							parser.clear_all();
						}
				}
			usleep(9000000);
		}
	while (parser.client::check_end());
	//end proper
	parser.clear_bloom_tickers();
	// parser.clear_ticker_retries();
	parser.data_base::finish_db();
	delete[] watcher;
}

int main(int argc, char **argv)
{
	// if no args listen the sirectory in the header for changes
	if (argc == 1)
		parse_from_watcher();
	// if a directory is given parse all the files inside
	else if (argc == 2)
		parse_from_dir(argv[1]);
	else
		printf("Nothing to do\n");
	return (1);
}
