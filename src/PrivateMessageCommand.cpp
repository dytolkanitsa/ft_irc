//
// Created by shaurmyashka on 10/5/21.
//

#include "PrivateMessageCommand.hpp"

void PrivateMessageCommand::execute(User *user) {

}

PrivateMessageCommand::PrivateMessageCommand() : Command(std::array<std::string, 6>()) {
	this->name = "PRIVMSG";
}
