//
// Created by shaurmyashka on 9/19/21.
//
#include <cstring>

#include "Server.hpp"

/**
 *separate the numbers of the first argument delimited ':'
 * @param av first argument from main [host:port_network:password_network]
 * @return allocated array of std::strings[3] or nullptr if av is wrong type
 */
std::string * getArgs(const std::string& av){
	std::string *result = new std::string[3];
	size_t pos = 0;
	size_t newPos;

	for (int i = 0; i < 3; i++){
		newPos = av.find(':', pos);
		if (newPos == std::string::npos && i != 2 || (i == 2 && newPos != std::string::npos))
			return nullptr;
		result[i] = av.substr(pos, newPos - pos);
		pos = newPos + 1;
	}
	return result;
}

int main(int ac, char ** av){
	if (ac == 3){
		av[2][strlen(av[2] -1)] = '\0';
		Server server(nullptr, av[1], av[2]);
	} else if (ac == 4){
		std::string *res = getArgs(av[1]);
		if (res == nullptr){
			std::cout << "./ircserv [host:port_network:password_network] <port> <password>\n";
			return 1;
		}
//		std::string * port
		Server server(&res[0], av[2], av[3]); // todo: какие аргументы мы передаем?
		server.init();
		server.start();
		// так как выделена память на первый аргумент и массив в принципе, надо не забыть ее освободить
	} else
	{
		std::cout << "./ircserv [host:port_network:password_network] <port> <password>\n";
		return 1;
	}
}
