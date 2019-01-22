#******************************************************************************#
#                                                                              #
#                                                                              #
#    Makefile                                                                  #
#                                                                              #
#    By: Mateo <teorodrip@protonmail.com>                                      #
#                                                                              #
#    Created: 2019/01/04 17:47:52 by Mateo                                     #
#    Updated: 2019/01/22 09:49:19 by Mateo                                     #
#                                                                              #
#******************************************************************************#

.PHONY: all clean fclean re

SHELL = /bin/bash

NAME = parser

C_FLAGS = -Wall -Werror -Wextra -g -lxlsxio_read -lpq

CC = g++

FUNCS = client.cpp \
		dir_watcher.cpp \
		parser.cpp \
		data_base.cpp \
		main.cpp

SRCS_DIR = srcs/

INC_DIR = includes/

OBJ_DIR = objects/

OBJ = $(patsubst %.cpp, $(OBJ_DIR)%.o, $(FUNCS))

INC = $(wildcard $(INC_DIR)*.h)

I := 1

N_SRCS = $(shell ls srcs | wc -l)

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(OBJ) -o $(NAME) $(C_FLAGS)

$(OBJ_DIR)%.o: $(SRCS_DIR)%.cpp $(INC)
	@mkdir -p $(OBJ_DIR)
	@echo -n "Compiling [$(shell echo ${I})/${N_SRCS}] => $(@F)"
	@if $(CC) -c -I $(INC_DIR) $< -o $@ $(C_FLAGS) ; then \
		 echo	" ===>[OK]"; \
	 fi
	@$(eval I=$(shell echo $$(($(I)+1))))

clean:
	@echo "***Cleaning Objects***"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "***Cleaning Executables & Libraries***"
	@rm -f $(NAME)

re: fclean
	@make
