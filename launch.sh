#!/bin/bash

make re;
make re -C ./parser;

./vm_launcher;

wait;

./parser/parser;

make fclean;
make fclean -C ./parser;
