#ifndef FT_IRC_CHANNEL_HPP
#define FT_IRC_CHANNEL_HPP

#include <iostream>
#include <vector>
//#include "User.hpp"

class User;

class Channel
{
private:
	std::vector <User *> users;
	std::string			channelName;

public:
	Channel(std::string chName);
	virtual ~Channel();

	std::string			getChannelName(void);
	std::vector<User *>	getUsers(void);
	User*				getUser(std::string	userName);

	bool	ifUserExist(std::string userName);
	void	setUser(User* user);
	void	sendMessageToChannel(std::string msg, User *sender);
	void	removeUser(std::string userName);

};

#endif