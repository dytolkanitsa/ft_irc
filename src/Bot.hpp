//
// Created by shaurmyashka on 10/24/21.
//

#ifndef FT_IRC_BOT_HPP
#define FT_IRC_BOT_HPP

#include <iostream>
#include "User.hpp"

class User;

class Bot : public User{
private:
	std::string host;
	std::string port;
public:
	Bot(int socketFd, std::string host, std::string port);
//	~Bot();
	void startBot();
};


#endif //FT_IRC_BOT_HPP
