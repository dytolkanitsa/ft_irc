//
// Created by shaurmyashka on 10/24/21.
//

#include "Bot.hpp"

int main(int argc, char *argv[])
{
	if (argc != 4){
		std::cout << "args error: ./bot <host> <port> <pass>" << std::endl;
		return 1;
	}
	Bot bot(-1, argv[1], argv[2], argv[3]);
	bot.startBot();
	return 0;
}
