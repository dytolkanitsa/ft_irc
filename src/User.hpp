//
// Created by shaurmyashka on 10/1/21.
//

#ifndef FT_IRC_USER_HPP
#define FT_IRC_USER_HPP

#include <iostream>
#include <sys/socket.h>


class User {
private:
	int socketFd;
	std::string nickName;
	std::string realName;
	std::string message;
	std::string password;
	// bool	enterPassword;
	// bool	registered;
	 bool	isOperator;

public:
	User(int socketFd, char *host, int port);
	virtual ~User();

	void	messageToUser(std::string msg);

	std::string 	getNickName(void);
	int 			getSocketFd(void);
	std::string 	getMessage(void);
	std::string 	getRealName(void);
	std::string		getPassword();

	// bool	getEnterPassword(void) {return enterPassword;} // 7
	// bool	getRegistered(void) {return registered;} // 8
	// bool	getIsOperator(void) {return isOperator;} // 9

	void	setNickName(std::string nickName);
	void	setSocketFd(int sockt);
	void	setRealName(std::string realName);
	void 	setMessage(std::string message);
	void	setPassword(std::string pass);
	void 	makeOperator();
	void 	removeOperator();
	// void	setEnterPassword(bool param) {return enterPassword = param;} // 7
	// void	setRegistered(bool param) {return registered = param;} // 8
	// void	setIsOperator(bool param) {return isOperato = param;} // 9

};


#endif //FT_IRC_USER_HPP
