//
// Created by shaurmyashka on 10/1/21.
//

#include "User.hpp"

User::User(int socketFd) : socketFd(socketFd), registered(false), enterPassword(false), isOperator(false), nickName("*"){}

User::~User() {
}

std::string	User::getNickName() const {
	return nickName;
}

int	User::getSocketFd() const {
	return socketFd;
}

std::string 	User::getRealName() {
	return realName;
}

void	User::setNickName(const std::string & nickName) {
	this->nickName = nickName;
}

void	User::setSocketFd(int socket) {
	this->socketFd = socket;
}

void	User::setRealName(const std::string & realName) {
	this->realName = realName;
}

void User::setPassword(const std::string & pass) {
	this->password = pass;
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

void User::messageToUser(const std::string & msg) const {
	send(this->socketFd, msg.c_str(), msg.length(), 0);
}
