//
// Created by shaurmyashka on 10/1/21.
//

#include "User.hpp"

User::User(int socketFd, char *host, int port) : socketFd(socketFd), host(host), port(port){

}

User::~User() {

}
