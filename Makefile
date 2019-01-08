#******************************************************************************#
#                                                                              #
#                                                                              #
#    Makefile                                                                  #
#                                                                              #
#    By: Mateo <teorodrip@protonmail.com>                                      #
#                                                                              #
#    Created: 2019/01/02 13:35:10 by Mateo                                     #
#    Updated: 2019/01/08 15:28:40 by Mateo                                     #
#                                                                              #
#******************************************************************************#

.PHONY: all clean fclean re

SHELL = /bin/bash

NAME = vm_launcher

C_FLAGS = -Wall -Werror -Wextra -lpthread -lpq -g

CC = gcc

FUNCS =	data_base.c \
		server.c \
		communication.c \
		invert_bytes.c \
		main.c

SRCS_DIR = launcher/srcs/

INC_DIR = launcher/includes/

OBJ_DIR = launcher/objects/

OBJ = $(patsubst %.c, $(OBJ_DIR)%.o, $(FUNCS))

INC = $(wildcard $(INC_DIR)*.h)

I := 1

N_SRCS = $(shell ls srcs | wc -l)

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(OBJ) -o $(NAME) $(C_FLAGS)

$(OBJ_DIR)%.o: $(SRCS_DIR)%.c $(INC)
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
