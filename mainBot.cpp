//
// Created by shaurmyashka on 10/24/21.
//

#include "Bot.hpp"

int main(int argc, char *argv[])
{
	if (argc != 3){
		std::cout << "args error: ./bot <host> <port>" << std::endl;
		return 1;
	}
	Bot bot(-1, argv[1], argv[2]);
	bot.startBot();
	return 0;
}
