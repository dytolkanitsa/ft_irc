//
// Created by shaurmyashka on 10/7/21.
//

#include "Guest.hpp"

Guest::Guest(int socketFd, char *host, int port) : socketFd(socketFd), host(host), port(port) {
}

bool Guest::isFullyRegister() {
	if (!this->nickName.empty() && !this->realName.empty() && !this->password.empty()){
		return true;
	} else {
		return false;
	}
}

Guest::~Guest() {
}

int Guest::getSoketFd() const {
	return this->socketFd;
}

int	Guest::getPort(void){
	return port;
}

std::string	Guest::getHost(void) {
	return host;
}

std::string	Guest::getNickName(void) {
	return nickName;
}

std::string	Guest::getRealName(void) {
	return realName;
}

std::string	Guest::getPassword(void) {
	return password;
}

std::string	Guest::getMessage(void) {
	return message;
}

void	Guest::setSocketFd(int sockt) {
	this->socketFd = sockt;
}

void	Guest::setPort(int port){
	this->port = port;
}

void	Guest::setHost(std::string host) {
	this->host = host;
}

void	Guest::setNickName(std::string nickName) {
	this->nickName = nickName;
}

void	Guest::setRealName(std::string realName) {
	this->realName = realName;
}

void	Guest::setPassword(std::string password) {
	this->password = password;
}

void 	Guest::setMessage(std::string message) {
	this->message = message;
}
