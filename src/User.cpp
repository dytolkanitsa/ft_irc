//
// Created by shaurmyashka on 10/1/21.
//

#include "Channel.hpp"
#include "User.hpp"

User::User(int socketFd) : socketFd(socketFd), registered(false), enterPassword(false), nickName("*"), enterNick(false) {}

User::~User() {
}

std::string	User::getNickName() /*const*/ {
	return nickName;
}

int	User::getSocketFd() /*const*/ {
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

void User::sendMessage(std::string msg)/* const*/ {
	send(this->socketFd, msg.c_str(), msg.length(), 0);
}

void User::addChannel(Channel *channel) {
	this->channels.push_back(channel);
}

void	User::setAwayMessage(std::string	message) {
	awayMessage = message;
}

std::string	User::getAwayMessage() {
	return awayMessage;
}

void User::leaveAllChannels() {
	for (int i = 0; i < this->channels.size(); i++){
		channels[i]->removeUser(this->nickName);
		channels.erase(channels.begin() + i);
	}
}
