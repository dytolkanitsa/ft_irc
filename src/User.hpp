//
// Created by shaurmyashka on 10/1/21.
//

#ifndef FT_IRC_USER_HPP
#define FT_IRC_USER_HPP

#include <iostream>

class User {
private:
	int socketFd;
	std::string nickName;
	std::string realName;
	std::string host;
	int port; //todo: uint32_t ?
	std::string message;
public:
	User(int socketFd, char *host, int port);
	virtual ~User();

	int getSocketFd();
	void setMessage(std::string newMessage);
	std::string getMessage();

	void setName(std::string name);
	std::string getName();

	void setRealName(std::string realName);
	std::string getRealName();

};


#endif //FT_IRC_USER_HPP
