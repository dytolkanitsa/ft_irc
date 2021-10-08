//
// Created by shaurmyashka on 10/7/21.
//

#include "Guest.hpp"

Guest::Guest(int socketFd, char *host, int port) : socketFd(socketFd), host(host), port(port) {


}

bool Guest::isFullyRegister() {
	if (!this->name.empty() && !this->realName.empty() && !this->password.empty()){
		return true;
	} else {
		return false;
	}
}

Guest::~Guest() {

}

int Guest::getFd() const {
	return this->socketFd;
}
