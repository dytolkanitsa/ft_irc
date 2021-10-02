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
