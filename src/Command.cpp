//
// Created by shaurmyashka on 10/5/21.
//

#include "Command.hpp"

Command::~Command() {

}
Command::Command(std::vector<User *> &users) : users(users) {

}

std::string Command::getName() {
	return this->name;
}

/**
 * разделяет строку по пробелам и устанавливает полученный массив, как значение поля для класса
 * @param argString строка-сообщение
 */
void Command::setArgs(std::string argString) {

	std::array<std::string, 6> args;
	std::string lastArg;
	size_t pos = 0;
	size_t newPos;

	newPos = argString.find(':', 0);
	if (newPos != std::string::npos){
		lastArg = argString.substr(newPos);
		argString.erase(newPos);
	}
	int i;
	for (i = 0; i < 6; i++){
		newPos = argString.find(' ', pos);
		if (newPos == std::string::npos)
		{
			args[i] = argString.substr(pos, newPos - pos);
			break;
		}
		args[i] = argString.substr(pos, newPos - pos);
		pos = newPos + 1;
	}
	if (!lastArg.empty())
		args[i] = lastArg;
	this->args = args;
}
