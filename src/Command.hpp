//
// Created by shaurmyashka on 10/5/21.
//

#ifndef FT_IRC_COMMAND_HPP
#define FT_IRC_COMMAND_HPP

#include <iostream>
#include <array>
#include <vector>
#include "User.hpp"

/**
 * абстрактный класс
 */
class Command {
protected:
	std::string name;
	std::vector<User *> &users;
	std::array<std::string , 6> args; //todo: 6  is max count of command args
public:
	Command(std::vector<User *> &users);
	virtual ~Command();
	virtual void execute(User *receiver, User *sender) = 0;
	std::string getName();
	void setArgs(std::string argString);
};


#endif //FT_IRC_COMMAND_HPP
