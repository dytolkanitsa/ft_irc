//
// Created by shaurmyashka on 10/1/21.
//

#include "User.hpp"

User::User(int socketFd) : socketFd(socketFd), registered(false), enterPassword(false), isOperator(false), nickName("*"){}

User::~User() {
}

std::string	User::getNickName() {
	return nickName;
}

int	User::getSocketFd() {
	return socketFd;
}

std::string 	User::getMessage() {
	return message;
}

std::string 	User::getRealName() {
	return realName;
}

void	User::setNickName(std::string nickName) {
	this->nickName = nickName;
}

void	User::setSocketFd(int sockt) {
	this->socketFd = sockt;
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
