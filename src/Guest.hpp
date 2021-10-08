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

	int 			getSocketFd() const;
	int				getPort(void);
	std::string 	getHost(void);
	std::string 	getNickName(void);
	std::string 	getRealName(void);
	std::string		getPassword(void);
	std::string 	getMessage(void);

	void	setSocketFd(int sockt);
	void	setPort(int port);
	void	setHost(std::string host);
	void	setNickName(std::string nickName);
	void	setRealName(std::string realName);
	void	setPassword(std::string password);
	void 	setMessage(std::string message);
	
};


#endif //FT_IRC_GUEST_HPP
