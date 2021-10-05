//
// Created by OUT-Podovalov-PG on 26.09.2021.
//

#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <array>
#include <arpa/inet.h>

#include "Server.hpp"
//#include "User.hpp"

Server::Server(const std::string *host, const std::string &port, const std::string &password)
: host(nullptr), port(port), password(password), socketFd(-1) {

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

	if ((status = getaddrinfo(this->host ? this->host->c_str() : nullptr, this->port.c_str(), &hints, &serverInfo)) != 0) {
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
		exit(1);
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
//_Noreturn
[[noreturn]] void Server::start() {
	if (listen(this->socketFd, 10) == -1){//todo: 10 - очередь из соединений
		exit(1);
		//todo: listen error
	}
	pollfd sPollfd {this->socketFd, POLLIN, 0};
	if (fcntl(this->socketFd, F_SETFL, O_NONBLOCK) == -1){
		exit(1);
		//todo: fcntl error
	}
	this->fds.push_back(sPollfd);
	std::vector<pollfd>::iterator	it;
	while(true){
		it = this->fds.begin();
		if (poll(&(*it), this->fds.size(), -1) == -1){
			exit(1);
			//todo: poll error
		}
	///после этого нужно что-то сделать с тем, что нам пришло после poll
		this->acceptProcess();
	}
}

Server::~Server() {

}

void Server::acceptProcess() {
//	std::vector<pollfd>::iterator itFds;
	pollfd nowPollfd;

	for (int i = 0; i < this->fds.size(); i++) {
		nowPollfd = this->fds[i];

		if ((nowPollfd.revents & POLLIN) == POLLIN){ ///модно считать данные

			if (nowPollfd.fd == this->socketFd) { ///accept
				std::cout << "🤔" << std::endl;
				int clientSocket;
				sockaddr_in		clientAddr;
				memset( &clientAddr, 0, sizeof(clientAddr));
				socklen_t socketLen = sizeof (clientAddr);

				clientSocket = accept(this->socketFd, (sockaddr *) &clientAddr, &socketLen);
				if (clientSocket == -1) {
					//todo: error accept
					continue;
					/*std::cout << strerror(errno) << std::endl;
					std::cout << strerror(EAGAIN) << std::endl;
					std::cout << strerror(EWOULDBLOCK) << std::endl;
					std::cout << strerror(EBADF) << std::endl;
					std::cout << strerror(ECONNABORTED) << std::endl;
					std::cout << strerror(EFAULT) << std::endl;
					std::cout << strerror(EINTR) << std::endl;
					std::cout << strerror(EINVAL) << std::endl;
					std::cout << strerror(EMFILE) << std::endl;
					std::cout << strerror(ENOBUFS) << std::endl;
					std::cout << strerror(ENOMEM) << std::endl;
					std::cout << strerror(ENOTSOCK) << std::endl;
					std::cout << strerror(EOPNOTSUPP) << std::endl;
					std::cout << strerror(EPERM) << std::endl;
					std::cout << strerror(EPROTO) << std::endl;
					exit(1);*/
				}

				pollfd clientPollfd {clientSocket, POLLIN, 0};
				this->fds.push_back(clientPollfd);

				if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1) {
					//todo: fcntl error
					throw std::runtime_error("fcntl error");
				}
					std::cout<< "cs:" << clientSocket << std::endl;
					std::cout<< "sfd:" << this->socketFd << std::endl;
					//нужно создать пользователя
					User *user = new User(clientSocket, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
					//а теперь добавить его в "массив" пользователей в сервере
					this->users.push_back(user);
//					break;
			}
			else { ///нужно принять данные не с основного сокета, который мы слушаем(клиентского?)
				try {
					std::cout << "fd: " << nowPollfd.fd << std::endl;
//					std::vector<User *>::iterator	itUser = users.begin();
//					std::advance(itUser, std::distance(fds.begin(), itFds) - 1);
					//				itUser->getSocketFd();
					recvMessage(this->users[i ? i - 1 : 0]);
					sendMessage(this->users[i ? i - 1 : 0]);
				} catch (std::runtime_error & e) {
					std::cout << e.what() << std::endl;
				}
			}
		}
		else if ((nowPollfd.revents & POLLHUP) == POLLHUP){ ///кто-то оборвал соединение
		}
	}
}

/**
 * функция для получения сообщения и добавления к нему "\r\n"
 * @param user класс пользователя для получения сообщения
 */
void Server::recvMessage(User *user) {
	char message[100]; /*todo: 100?*/
	ssize_t recvByte;
	memset(message, '\0', sizeof(message));
	recvByte = recv(user->getSocketFd(), message, sizeof(message), 0);
	if (recvByte <= 0){
		//todo: error;
//		throw std::runtime_error("recv < 0");
		return;
	}
	message[strlen(message) + 1] = '\r';
	message[strlen(message) + 2] = '\n';
	user->setMessage(message);
	std::cout << "💬 ➡ " << message << " ⬅ 🐢" << std::endl;
}

/**
 * разделяет строку по пробелам
 * @param argString строка-сообщение
 * @return std::array содержащий аргументы
 */
std::array<std::string, 6>getArgs(std::string argString){
	std::array<std::string, 6> args;
	size_t pos = 0;
	size_t newPos;

	for (int i = 0; i < 6; i++){
		newPos = argString.find(' ', pos);
		if (newPos == std::string::npos)
		{
			args[i] = argString.substr(pos, newPos - pos);
			return args;
		}
		args[i] = argString.substr(pos, newPos - pos);
		pos = newPos + 1;
	}
	return args;

}


/**
 * функция(временная) для отправки сообщения всем пользователям, кроме него самого
 * @param user класс пользователя, который отправляет сообщение
 */
void Server::sendMessage(User *user) {
	std::vector<User *>::iterator	itUser;
	User *curUser;
	std::array<std::string, 6> args;

	args = getArgs(user->getMessage());
	for (itUser = users.begin(); itUser != users.end(); itUser++){
		curUser = *itUser;
		/*if (*itUser == user){
			не отправляем сообщение
		} else */if (curUser != user){
			send(curUser->getSocketFd(), user->getMessage().c_str(), user->getMessage().length(), 0);
//			отправляем
		}
	}
}
