//
// Created by shaurmyashka on 10/7/21.
//

#ifndef FT_IRC_GUEST_HPP
#define FT_IRC_GUEST_HPP

#include <iostream>

class Guest {
private:
	int socketFd;
	int port;
	std::string host;
	std::string nickName;
	std::string realName;
	std::string password;
	std::string message;
public:
	Guest(int socketFd, char *host, int port);
	virtual ~Guest();
	bool isFullyRegister();
	int getFd() const;

	//сеттеры геттеры
	
};


#endif //FT_IRC_GUEST_HPP
