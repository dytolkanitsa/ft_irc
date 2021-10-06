//
// Created by shaurmyashka on 10/5/21.
//

#ifndef FT_IRC_USERCOMMAND_HPP
#define FT_IRC_USERCOMMAND_HPP

#include "Command.hpp"

class UserCommand : public Command{
public:
	UserCommand(std::vector<User *> &users);
	void execute(User *receiver, User *sender) override;
};


#endif //FT_IRC_USERCOMMAND_HPP
