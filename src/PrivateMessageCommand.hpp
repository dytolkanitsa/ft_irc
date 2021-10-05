//
// Created by shaurmyashka on 10/5/21.
//

#ifndef FT_IRC_PRIVATEMESSAGECOMMAND_HPP
#define FT_IRC_PRIVATEMESSAGECOMMAND_HPP

#include "Command.hpp"
#include "User.hpp"

class PrivateMessageCommand : public Command{
public:
	PrivateMessageCommand(std::vector<User *> &users);
	void execute(User *receiver, User *sender) override;
};


#endif //FT_IRC_PRIVATEMESSAGECOMMAND_HPP
