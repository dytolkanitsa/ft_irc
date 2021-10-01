//
// Created by shaurmyashka on 10/1/21.
//

#ifndef FT_IRC_USER_HPP
#define FT_IRC_USER_HPP

#include <iostream>

class User {
private:
	int socketFd;
	std::string name;
	std::string host;
	int port; //todo: uint32_t ?
public:
	User(int socketFd, char *host, int port);
	virtual ~User();

};


#endif //FT_IRC_USER_HPP
