NAME	=	ft_irc
BONUS = 	ft_bot

SRCS	:=	main.cpp\
			src/Server.cpp\
			src/User.cpp\
			src/Channel.cpp\
			src/Command.cpp\
		 	src/Message.cpp\
		  	src/OtherServerMethods.cpp\

HEAD	:=	src/Server.hpp\
			src/User.hpp\
			src/Channel.hpp\

BONUS_SRCS = bot/Bot.cpp\
             bot/mainBot.cpp\
             bot/User.cpp\
             bot/Channel.cpp

BONUS_HEADER = bot/Bot.hpp\
             bot/User.hpp\
             bot/Channel.hpp

COMP		= clang++ -Wall -Wextra -Werror -std=c++11

OBJS	=	$(SRCS:.cpp=.o)

BONUS_OBJS = $(BONUS_SRCS:.cpp=.o)

all:		$(NAME)

$(NAME):	$(OBJS)
			$(COMP) $(OBJS) -o $(NAME)

bonus:		bot $(NAME)

bot:		$(BONUS_OBJS)
			$(COMP) $(BONUS_OBJS) -o $(BONUS)

clean:
			rm -rf $(OBJS) $(BONUS_OBJS)

.cpp.o:
			$(COMP) -c $< -o $(<:.cpp=.o)

$(OBJS):	$(HEAD)

$(BONUS_OBJS):	$(BONUS_HEADER)

fclean:		clean
			rm -rf $(NAME) $(BONUS)

re:			fclean all

.PHONY:		all clean fclean re bonus
