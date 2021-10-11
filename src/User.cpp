//
// Created by shaurmyashka on 10/1/21.
//

#include "User.hpp"

User::User(int socketFd, char *host, int port) : socketFd(socketFd), host(host), port(port){
}

User::~User() {
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

std::string 	User::getMessage(void) {
	return message;
}

std::string 	User::getRealName(void) {
	return realName;
}

bool	getEnterPassword(void) {
	return enterPassword;
}

bool	getRegistered(void) {
	return registered;
}

bool	getIsOperator(void) {
	return isOperator;
}

void	User::setNickName(std::string nickName) {
	this->nickName = nickName;
}

void	User::setSocketFd(int sockt) {
	this->socketFd = sockt;
}

void	User::setHost(std::string host) {
	this->host = host;
}

void	User::setRealName(std::string realName) {
	this->realName = realName;
}

void 	User::setMessage(std::string message) {
	this->message = message;
}

void User::setPassword(std::string pass) {
	this->password = pass;
}

void	setEnterPassword(bool param) {
	return enterPassword = param;
}

void	setRegistered(bool param) {
	return registered = param;
}

void	setIsOperator(bool param) {
	return isOperato = param;
}

std::string User::getPassword() {
	return this->password;
}

void User::makeOperator() {
	this->isOperator = true;
}

void User::removeOperator() {
	this->isOperator = false;
}

void User::messageToUser(std::string msg) {
	send(this->socketFd, msg.c_str(), msg.length(), 0);
}
