# KEITH PARSER

This program consists of three independents working together.
## Server
The server will handle all the information that is needed by the clients, there can be two types of clients: The virtual machine that will compute the tickers, and the parser that will parse the output of the virtual machines. The server provides the ticker in batches to the virtual machines and the whole list of tickers to the parser. Also controls the workflow of those two, reading signals like the start of the virtual machine, or the signal that sends the parser when starts his work, in order to catch errors and retry if necessary (this is written and prepared but not implemented, because with the new template does not generate those errors, so it is written for reference, and implement in the next version in a modular way). The server can start either from a static list of tickers, or reading it from the data base, for the first case its activated atomatically if the compiler finds the file `tickers.h` inside `./launcher/includes/`, this file must follow the example in the repository, or can be generated with the script `tickers_serializer.sh` from a csv with two columns (bloomberg tickers, and capital iq).

For execute the server just type:
```
make && ./vm_launcher
```

If you want to start from specific ticker number, you can put it in the first argument:

```
make && ./vm_launcher 4234
```

## Excel VBA
This part is a client that will receive a batch of tickers and calculate them, for the instance, onece a virtual machine (or computer) is open, open with Excel the `capitaliq_db.xlsm` file and go to `Macros > Start` to connect to the server and start calculating tickers, this will save the output sheets inside `Z:\outputs_windows_[vm_id]\` directory, if those directories don't exist will probably generate a error (This will probably change in the new version, and put all the files inide just one directory, because there is no need to reboot, and the parser and the server will be just one executable, so will be a lot easyer to locate each virtual machine)

## Parser
This program will either take a directory and parse all the files inside or wait to a file to be written inside a directory, this will update automaticlly the data base with the tickers in the sheets, if there is an error in the SQL request will be printed, for the instance the only error that is not fatal, is the duplicate key error simply will, continue with the next tickers and do nothing in the db for this one, in other case the program will end.

For execute it to parse each file inside a directory:
```
make -C ./parser && ./parser/parser [path_to_directory]
```

For execute it, so it watches changes in a directory, must change the path inside the header file `./parser/includes/parser.hpp` and just type:

```
make -C ./parser && ./parser/parser
```

## Notes

you can also execute the `launch.sh` script to launch the server and the parser, for the moment, the clients will be executed in the desired computer or virtual machine.