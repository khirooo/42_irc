NAME	=	server

CC		=	g++
INCLUDES	=	./includes
FLAGS	=	 -I $(INCLUDES)

SRCDIR	=	src/
SRCFILES = 	Server.cpp  Client.cpp Message.cpp Reply.cpp Utils.cpp
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