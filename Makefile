NAME	=	server

CC		=	c++
INCLUDES	=	./includes
FLAGS	=	 -Wall -Werror -Wextra -std=c++98 -I $(INCLUDES)

SRCDIR	=	src/
SRCFILES = 	Server.cpp
SRCS	=	$(addprefix $(SRCDIR), $(SRCFILES))
SRCS	+= main.cpp

OBJS	=	$(SRCS:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJS)
		@echo "Compiling $@"
		@$(CC) $(FLAGS) $(OBJS) -o $@

%.o: %.cpp
	@$(CC) $(FLAGS) -c $< -o $@

clean: 
	@rm -rf $(OBJS)

fclean: clean
		@rm -rf $(NAME)


re: fclean all

.PHONY: all clean fclean re