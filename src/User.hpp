//
// Created by shaurmyashka on 10/1/21.
//

#ifndef FT_IRC_USER_HPP
#define FT_IRC_USER_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h>
//#include "Channel.hpp"

class Channel;


class User {
private:
	int socketFd;
	std::string nickName;
	std::string realName;
	std::string password;
	std::string	awayMessage;
	bool	enterPassword;
	bool	registered;
	bool	isOperator;
	std::vector<Channel *> channels;

public:
	User(int socketFd);
	virtual ~User();

	void	messageToUser(const std::string & msg)/* const*/;

	std::string 	getNickName() /*const*/;
	int 			getSocketFd() /*const*/;
	std::string 	getRealName();
	std::string		getPassword();

	bool	getEnterPassword() const {return enterPassword;} // 7
	bool	getRegistered() const {return registered;} // 8
	bool	getIsOperator() const {return isOperator;} // 9

	void	setNickName(const std::string & nickName);
	void	setSocketFd(int socket);
	void	setRealName(const std::string & realName);;
	void	setPassword(const std::string & pass);
	void 	makeOperator();
	void 	removeOperator();
	void	setEnterPassword(bool param) {enterPassword = param;} // 7
	void	setRegistered(bool param) {registered = param;} // 8
	void	setIsOperator(bool param) {isOperator = param;} // 9

	void	addChannel(Channel *channel);

	void	setAwayMessage(std::string	message);
	std::string 	getAwayMessage();
	 void leaveAllChannels();
};


#endif //FT_IRC_USER_HPP
