//
// Created by shaurmyashka on 10/1/21.
//

#include "User.hpp"

User::User(int socketFd, char *host, int port) : _socketFd(socketFd), _host(host), _port(port){
}

User::~User() {
}

void	messageToUser(std::string msg)
{
	send(this->socketFd, msg.c_str(), msg.length(), 0);
}

std::string	User::getNickName(void) {
	return nickName;
}

int	User::getSocketFd(void) {
	return socketFd;
}

std::string	User::getHost(void) {
	return host;
}

int	User::getPort(void) {
	return port;
}

std::string 	User::getMessage(void) {
	return message;
}

std::string 	User::getRealName(void) {
	return realName;
}

void	User::setNickName(std::string nickName) {
	return this->nickName = nickName;
}

void	User::setSocketFd(int sockt) {
	return this->socketFd = sockt;
}

void	User::setHost(std::string host) {
	return this->host = host;
}

void	User::setPort(int port) {
	return this->port = port;
}

void	User::setRealName(std::string realName) {
	return this->realName = realName;
}

void 	User::setMessage(std::string message) {
	return this->message = message;
}