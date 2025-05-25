NAME		= philo

CC			= gcc
CFLAGS		= -Wall -Wextra -Werror -g
RM			= rm -f

SRCS		= main.c inits.c time_management.c utils_problem.c utils_string.c philosopher_routine.c philosopher_monitor.c

OBJS		= $(SRCS:.c=.o)

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CC) $(CFLAGS) -o $(NAME) $(OBJS) -pthread

clean:
			$(RM) $(OBJS)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re