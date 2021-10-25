//
// Created by shaurmyashka on 10/1/21.
//

#ifndef FT_IRC_USER_HPP
#define FT_IRC_USER_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h>

class Channel;


class User {
private:
	int socketFd;
	std::string nickName;
	std::string realName;
	std::string awayMessage;
	bool enterPassword;
	bool enterNick;
	bool registered;
	std::vector<Channel *> channels;

public:
	User(int socketFd);

	virtual         ~User();

	void sendMessage(std::string msg);

	std::string getNickName();

	int getSocketFd();

	bool getEnterPassword() const { return enterPassword; }
	bool getEnterNick() const { return enterNick; }

	bool getRegistered() const { return registered; }

	void setNickName(const std::string &nickName);

	void setRealName(const std::string &realName);

	void setEnterPassword(bool param) { enterPassword = param; }
	void setRegistered(bool param) { registered = param; }
	void setEnterNick(bool param) { enterNick = param; }

	void addChannel(Channel *channel);

	void setAwayMessage(std::string message);

	std::string getAwayMessage();

	void leaveAllChannels();
};


#endif //FT_IRC_USER_HPP
