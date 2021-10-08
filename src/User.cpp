//
// Created by shaurmyashka on 10/1/21.
//

#include "User.hpp"

User::User(int socketFd, char *host, int port) : _socketFd(socketFd), _host(host), _port(port){
}

User::~User() {
}


std::string	getNickName(void) {
	return nickName;
}

int	getSocketFd(void) {
	return socketFd;
}

std::string	getHost(void) {
	return host;
}

int	getPort(void) {
	return port;
}

std::string 	getMessage(void) {
	return message;
}

std::string 	getRealName(void) {
	return realName;
}

void	setNickName(std::string nickName) {
	return this->nickName = nickName;
}

void	setSocketFd(int sockt) {
	return this->socketFd = sockt;
}

void	setHost(std::string host) {
	return this->host = host;
}

void	setPort(int port) {
	return this->port = port;
}

void	setRealName(std::string realName) {
	return this->realName = realName;
}

void 	setMessage(std::string message) {
	return this->message = message;
}