MAKEFLAGS	= --silent
GREEN		=	\033[3;32m
DEFAULT		=	\033[0m

NAME		= 	ircserv
SOURCESPATH	= 	./
SOURCESLIST	= 	main.cpp channel.cpp client.cpp server.cpp \
				join.cpp invite.cpp kick.cpp part.cpp quit.cpp \
				privmsg.cpp mode.cpp topic.cpp
SOURCES = $(addprefix $(SOURCESPATH), $(SOURCESLIST))
OBJECTS	= $(SOURCES:.cpp=.o)

COMPILE	= c++
FLAGS	= -Wall -Wextra -Werror -O3 -std=c++98
# Replaced -g with -O3 to optimise the code (removing debug symbols).
# -O3 is faster than -g and -O2, but it takes longer to compile.
VALGRIND	= valgrind -s --leak-check=full --show-leak-kinds=all
# -s to suppress the summary, -v to show the full report.
# --track-origins=yes to show where the memory was allocated.
REMOVE	= rm -f

%.o: %.cpp server.hpp client.hpp channel.hpp
	$(COMPILE) $(FLAGS) -c $< -o $@

all:	$(NAME)

$(NAME):
	$(COMPILE) $(FLAGS) $(SOURCES) -o $(NAME)

clean:
	@$(REMOVE) $(OBJECTS)

fclean: clean
	@$(REMOVE) $(NAME)
	echo "$(GREEN)Limpíssimo!$(DEFAULT)"

re:		fclean all

#6667 is the default port for IRC. 6697 for TLS.
run:	$(NAME)
	./$(NAME) 6667 senha42
# Open a new terminal and type:
# nc localhost 6667
# PASS 53nh4P35504L
# NICK WicCaesar
# USER cnascime 0 * César Augusto

vrun:	$(NAME)
	$(VALGRIND) ./$(NAME) 6667 senha42

rerun:	re run

.PHONY:		all clean fclean re
