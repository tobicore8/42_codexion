NAME	= codexion

CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread

SRCS	= codexion.c parse.c time.c sim_setup.c dongle_setup.c \
		  coder_setup.c heap.c heap_utils.c log.c stop.c dongle.c \
		  coder.c monitor.c
OBJS	= $(SRCS:.c=.o)
HEADER	= codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
