//
// Created by OUT-Podovalov-PG on 26.09.2021.
//

#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "Server.hpp"

Server::Server(const std::string *host, const std::string &port, const std::string &password)
: host(host), port(port), password(password), socketFd(-1) {

}

/**
 * создание структуры addrinfo, создание сокета и bind
 */
void Server::init() {
	int status;
	int socketFd;
	int yes = 1;
	struct addrinfo hints, *serverInfo, *rp;

	memset(&hints, 0, sizeof hints); // убедимся, что структура пуста
	hints.ai_family = AF_UNSPEC;     // неважно, IPv4 или IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream-sockets
	hints.ai_flags = AI_PASSIVE;     // заполните мой IP-адрес за меня

	if ((status = getaddrinfo(this->host->c_str(), this->port.c_str(), &hints, &serverInfo)) != 0) {
		// TODO: errors
	}

	/*
	 * getaddrinfo() returns a list of address structures.
	 * Try each address until we successfully bind(2).
	 * If socket(2) (or bind(2)) fails, we (close the socket and) try the next address.
	*/

	for (rp = serverInfo; rp != nullptr; rp = rp->ai_next) {
		socketFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (socketFd == -1) {
			continue;
		}

		if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			// TODO: errors
		}

		if (bind(socketFd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break; // Success
		}
		close(socketFd);
	}
	if (rp == nullptr)  {
//		fprintf(stderr, "server: failed to bind");
		// todo: bind error
//		return 2;
	}
	freeaddrinfo(serverInfo); // освобождаем связанный список
	this->socketFd = socketFd;
}
/**
 * listen сокета, создание pollfd структуры для этого сокета,
 * добавление в вектор структур, и главный цикл
 */
_Noreturn
void Server::start() {
	if (listen(this->socketFd, 10) == -1){//todo: 10 - очередь из соединений
		//todo: listen error
	}
	pollfd sPollfd {socketFd, POLLIN, 0};
	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1){
		//todo: fcntl error
	}
	fds.push_back(sPollfd);
	std::vector<pollfd>::iterator	it;
	while(true){
		it = fds.begin();
		if (poll(&(*it), fds.size(), -1) == -1){
			//todo: poll error
		}
	///после этого нужно что-то сделать с тем, что нам пришло после poll
//		this->acceptProcess();
	///пока закоментированно, но предположительно будет вызываться вот этот метод
	}
}

Server::~Server() {

}

void Server::acceptProcess() {
	std::vector<pollfd>::iterator itFds;
	pollfd nowPollfd;

for (itFds = fds.begin(); itFds != fds.end(); itFds++){

	nowPollfd = *itFds;

	if ((nowPollfd.revents & POLLIN) == POLLIN){ ///модно считать данныке

		if (nowPollfd.fd == socketFd){ ///accept

			int clientSocket;
			sockaddr_in		clientAddr;
			socklen_t socketLen = sizeof (clientAddr);

			clientSocket = accept(socketFd, (sockaddr *) &clientAddr, &socketLen);
			if (clientSocket == -1){
				//todo: error accept
			}

			pollfd clientPollfd {clientSocket, POLLIN, 0};
			fds.push_back(clientPollfd);

			if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1){
				//todo: fcntl error
			}

		} else{ ///нужно принять данные не с основного сокета, который мы слушаем(клиентского?)
		}
	}
	else if ((nowPollfd.revents & POLLHUP) == POLLHUP){ ///кто-то оборвал соединение
	}
}


}





