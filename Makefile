NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
SRC = src/main.c src/parse.c src/time.c src/heap.c src/simulation.c \
	src/resource.c src/worker.c src/monitor.c
OBJ = $(SRC:.c=.o)

all: $(NAME)

$(OBJ): src/codexion.h

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
