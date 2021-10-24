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
#include "User.hpp"
#include "Channel.hpp"

#define GREEN "\033[32m"
#define STD "\033[0m"

Server::Server(const std::string *host, const std::string &port, const std::string &password)
		: host(nullptr), port(port), password(password), socketFd(-1) {
	commands = {&Server::passCommand, &Server::userCommand, &Server::nickCommand, &Server::privmsgCommand, &Server::joinCommand, &Server::listCommand, &Server::noticeCommand, &Server::awayCommand, &Server::quitCommand, &Server::partCommand,
				&Server::topicCommand, &Server::kickCommand};
	commandsName = {"PASS", "USER", "NICK", "PRIVMSG", "JOIN", "LIST", "NOTICE", "AWAY", "QUIT", "PART", "TOPIC", "KICK"};
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
		throw std::runtime_error("getaddrinfo error");
		exit(1);
	}
	for (rp = serverInfo; rp != nullptr; rp = rp->ai_next) {
		socketFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (socketFd == -1) {
			continue;
		}
		if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			throw std::runtime_error("setsockopt error");
		}
		if (bind(socketFd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break; // Success
		}
		close(socketFd);
	}
	if (rp == nullptr) {
		throw std::runtime_error("bind error");
	}
	freeaddrinfo(serverInfo); // освобождаем связанный список
	this->socketFd = socketFd;
}

/**
 * listen сокета, создание pollfd структуры для этого сокета,
 * добавление в вектор структур, и главный цикл
 */
[[noreturn]] void Server::start() {
	if (listen(this->socketFd, 10) == -1) {
		throw std::runtime_error("listen error");
	}
	pollfd sPollfd{this->socketFd, POLLIN, 0};
	if (fcntl(this->socketFd, F_SETFL, O_NONBLOCK) == -1) {
		throw std::runtime_error("fcntl error");
	}
	this->fds.push_back(sPollfd);
	std::vector<pollfd>::iterator it;
	while (true) {
		it = this->fds.begin();
		if (poll(&(*it), this->fds.size(), -1) == -1) {
			throw std::runtime_error("poll error");
		}
		///после этого нужно что-то сделать с тем, что нам пришло после poll
		this->acceptProcess();
	}
}

Server::~Server() {

}

void Server::acceptProcess() {
	pollfd nowPollfd;

	for (int i = 0; i < this->fds.size(); i++) {
		nowPollfd = this->fds[i];

		if ((nowPollfd.revents & POLLIN) == POLLIN) { ///модно считать данные

			if (nowPollfd.fd == this->socketFd) { ///accept
				int clientSocket;
				sockaddr_in clientAddr;
				memset(&clientAddr, 0, sizeof(clientAddr));
				socklen_t socketLen = sizeof(clientAddr);
				clientSocket = accept(this->socketFd, (sockaddr *) &clientAddr, &socketLen);
				if (clientSocket == -1) {
					continue;
				}
				pollfd clientPollfd{clientSocket, POLLIN, 0};
				this->fds.push_back(clientPollfd);
				if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1) {
					throw std::runtime_error("fcntl error");
				}
				std::cout << "cs:" << clientSocket << std::endl;
				std::cout << "sfd:" << this->socketFd << std::endl;
				User *user = new User(clientSocket);
				this->users.push_back(user);
			} else { ///нужно принять данные не с основного сокета, который мы слушаем(клиентского?)
				try {
					std::cout << "fd: " << nowPollfd.fd << std::endl;
					User *curUser = findUserByFd(nowPollfd.fd);
					this->commandProcess(*curUser, recvMessage(curUser->getSocketFd()));
				} catch (std::runtime_error &e) {
					std::cout << e.what() << std::endl;
				}
			}
		}
		if ((nowPollfd.revents & POLLHUP) == POLLHUP) { ///кто-то оборвал соединение
			User *user = findUserByFd(nowPollfd.fd);
			if (user == nullptr) {
				continue; // я вот вообще честно говоря вообще не знаю, когда такое может произойти
			}
			this->removeUser(user);
			close(fds[i].fd);
			this->fds.erase(fds.begin() + i);
		}
	}
}

/**
 * находит какую команду вызывает Юзер и выполняет ее
 * @param user указатель на юзера, который отправил сообщение
 */
void Server::commandProcess(User &user, const std::string &message) {

	std::vector<std::string> args = setArgs(message);
	if (args.empty()) {
		return;
	}
	std::string command = args[0];
	args.erase(args.begin());
	try {
		for (int i = 0; i < 12; i++) {
			if (command == this->commandsName[i]) {
				(this->*commands[i])(args, user);
				break;
			}
		}
	} catch (std::string &error) {
		user.sendMessage(error);
	}
}
