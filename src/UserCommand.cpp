//
// Created by shaurmyashka on 10/5/21.
//

#include "UserCommand.hpp"

UserCommand::UserCommand() : Command(<#initializer#>), Command(<#initializer#>) {
	this->name = "USER";
}

void UserCommand::execute(User *receiver, User *sender) {
	if (args.size() != 5){//User <username> <hostname> <servername> <realname>
		//todo: error of args
	} else {
		sender->setName(args[1]);
		sender->setRealName(args[4]);
	}
}
