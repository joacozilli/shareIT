COMPILER = gcc
FLAGS = -Wall -Wextra -g -Isrc -Iinclude
EXEC = shareIT
CONFIG_FILE_PATH = "config.json"

SRC = \
	src/str.c \
	src/data_structures/array/array.c \
	src/data_structures/avl_tree/avl_tree.c \
	src/network.c \
	src/events.c \
	main.c

# compile program
build:
	$(COMPILER) $(FLAGS) $(SRC) -o $(EXEC)

# delete executable
clean:
	rm $(EXEC)

# read config file
read-conf:
	cat $(CONFIG_FILE_PATH)