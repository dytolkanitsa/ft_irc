//
// Created by shaurmyashka on 10/1/21.
//

#include "User.hpp"

User::User(int socketFd, char *host, int port) : socketFd(socketFd), host(host), port(port){

}

User::~User() {

}

int User::getSocketFd() {
	return socketFd;
}

void User::setMessage(std::string newMessage) {
	this->message = newMessage;
}

std::string User::getMessage() {
	return message;
}

void User::setName(std::string name) {
	this->nickName = name;
}

std::string User::getName() {
	return this->nickName;
}

void User::setRealName(std::string realName) {
	this->realName = realName;
}

std::string User::getRealName() {
	return this->realName;
}
