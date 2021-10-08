//
// Created by shaurmyashka on 10/1/21.
//

#ifndef FT_IRC_USER_HPP
#define FT_IRC_USER_HPP

#include <iostream>

class User {
private:
	int 		socketFd;
	std::string nickName;
	std::string realName;
	std::string host;
	int 		port; //todo: uint32_t ?
	std::string message;

	// bool	enterPassword;
	// bool	registered;
	// bool	isOperator;

public:
	User(int socketFd, char *host, int port);
	virtual ~User();

	// void	messageToUser(std::string str);

	std::string 	getNickName(void);
	int 			getSocketFd(void);
	std::string 	getHost(void);
	int				getPort(void);
	std::string 	getMessage(void);
	std::string 	getRealName(void);

	// bool	getEnterPassword(void) {return enterPassword;} // 7
	// bool	getRegistered(void) {return registered;} // 8
	// bool	getIsOperator(void) {return isOperator;} // 9

	void	setNickName(std::string nickName);
	void	setSocketFd(int sockt);
	void	setHost(std::string host);
	void	setPort(int port);
	void	setRealName(std::string realName);
	void 	setMessage(std::string message);

	// void	setEnterPassword(bool param) {return enterPassword = param;} // 7
	// void	setRegistered(bool param) {return registered = param;} // 8
	// void	setIsOperator(bool param) {return isOperato = param;} // 9

};


#endif //FT_IRC_USER_HPP
