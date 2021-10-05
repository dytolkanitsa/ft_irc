//
// Created by shaurmyashka on 10/5/21.
//

#ifndef FT_IRC_COMMAND_HPP
#define FT_IRC_COMMAND_HPP

#include <iostream>
#include <array>
#include "User.hpp"

/**
 * абстрактный класс
 */
class Command {
protected:
	std::string name;
	std::array<std::string , 6> args; //todo: 6  is max count of command args
public:
	Command(std::array<std::string, 6> argsArray);
	virtual ~Command();
	virtual void execute(User *user) = 0;
};


#endif //FT_IRC_COMMAND_HPP
