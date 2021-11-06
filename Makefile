NAME	=	ft_irc
BONUS = 	ft_irc_bonus

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
             bot/Bot.hpp\
             bot/mainBot.cpp\

COMP		= clang++ -Wall -Wextra -Werror

OBJS	=	$(SRCS:.cpp=.o)

BONUS_OBJS = $(BONUS_SRCS:.cpp=.o)

bonus: $(BONUS)

all:		$(NAME)

$(NAME):	$(OBJS)
			$(COMP) $(OBJS) -o $(NAME)

$(BONUS): $(BONUS_OBJS)
	@$(COMP)  -o $(BONUS) $(BONUS_OBJS)
#	@#$(G++)  bot.cpp -o client


.cpp.o:
			$(COMP) -c $< -o $(<:.cpp=.o)

clean:
			rm -rf $(OBJS)

$(OBJS):	$(HEAD)

fclean:		clean
			rm -rf $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
