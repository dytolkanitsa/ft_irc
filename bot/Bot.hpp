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
	std::string pass;
public:
	Bot(int socketFd, std::string host, std::string port, std::string pass);
//	~bot();
	void startBot();
	void doRegister();
	void commandProcess(std::string arg);
};


#endif //FT_IRC_BOT_HPP
