#!/bin/bash

FILE_PATH="./tickers_keith_parser.csv";
OUT_FILE="./launcher/includes/tickers.h"
IFS=',';
declare -a tick_bbg=();
declare -a tick_capiq=();
declare -i n_tuples=-1;

touch $OUT_FILE

printf "#ifndef TICKERS_H\n#define TICKERS_H\n\n#define N_COLS 2\n\n#define TICK_LEN {" > $OUT_FILE

while read -r col1 col2
do
	if (($n_tuples == -1)); then
		n_tuples=0;
		continue
	fi
	len1=$(echo $col1 | wc -c);
	len2=$(echo $col2 | wc -c);
	printf "{$len1, $len2}, " >> $OUT_FILE
	tick_bbg+=($col1);
	tick_capiq+=($col2);
	n_tuples=$((n_tuples + 1))
done < $FILE_PATH

sed -i "$ s/..$//" $OUT_FILE

printf "}\n\n#define N_TUPLES $n_tuples\n\n#define TICKERS {" >> $OUT_FILE

for i in "${!tick_capiq[@]}";
do
	printf "{\"${tick_bbg[$i]}\", \"${tick_capiq[$i]}\"}, " >> $OUT_FILE;
done

sed -i "$ s/..$//" $OUT_FILE
printf "}\n\n#endif" >> $OUT_FILE
