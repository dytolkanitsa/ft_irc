//
// Created by shaurmyashka on 10/5/21.
//

#include "PrivateMessageCommand.hpp"

/**
 *	принимает класс получателя и отправителя и отправляет приватное сообщение
 * @param receiver класс получателя(возможно еще нужен и класс отправителя, но я не верена)
 * @param sender класс отправителя
 */
//todo: отправка пока только одному пользователю
void PrivateMessageCommand::execute(User *receiver, User *sender) {
	if (args.size() != 3){
		//todo: error (пока 3)
	}
}

PrivateMessageCommand::PrivateMessageCommand(std::vector<User *> &users)
		: Command(users) {
	this->name = "PRIVMSG";
}
