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
//	commands.push_back(new PrivateMessageCommand(this->users));
//	commands.push_back(new UserCommand(this->users));
	//todo: add commands
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
				//User *user = new User(clientSocket, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				//а теперь добавить его в "массив" пользователей в сервере
				//this->users.push_back(user);
				//break;
				// сейчас мы сначла создаем гостя
				Guest *guest = new Guest(clientSocket, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				this->guests.push_back(guest);
			}
			else { ///нужно принять данные не с основного сокета, который мы слушаем(клиентского?)
				try {
					std::cout << "fd: " << nowPollfd.fd << std::endl;
					User * curUser = findUserByFd(nowPollfd.fd);
					if (curUser == nullptr){
						Guest * curGuest = findGuestByFd(nowPollfd.fd);
						curGuest->setMessage(recvMessage(curGuest->getSocketFd()));
					}
					curUser->setMessage(recvMessage(curUser->getSocketFd()));
					///нуно найти это фдшник юзера или гостя
					//this->findUserByFd(nowPollfd.fd);
					//this->findGuestByFd(nowPollfd.fd);

					//после этого вся остальная шняга
//					std::vector<User *>::iterator	itUser = users.begin();
//					std::advance(itUser, std::distance(fds.begin(), itFds) - 1);
					//				itUser->getSocketFd();
//					sendMessage(this->users[i ? i - 1 : 0]);
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
std::string Server::recvMessage(int fd) {
	char message[100]; /*todo: 100?*/
	ssize_t recvByte;
	memset(message, '\0', sizeof(message));
	recvByte = recv(fd, message, sizeof(message) -2, 0);//потому что + "\r\n"
	if (recvByte <= 0){
		//todo: error;
//		throw std::runtime_error("recv < 0");
	}
	message[strlen(message) + 1] = '\r';
	message[strlen(message) + 2] = '\n';
	std::cout << "💬 ➡ " << message << " ⬅ 🐢" << std::endl;
	return (message);
}




/**
 * функция(временная) для отправки сообщения всем пользователям, кроме него самого
 * @param user класс пользователя, который отправляет сообщение
 */
void Server::sendMessage(User *user) {
//	std::string message = user->getMessage();
//	std::string command = message.substr(0,message.find(' '));
//	Command *curCommand;
//	curCommand = this->findCommandByName(command);
//	curCommand->setArgs(message);
	std::vector<User *>::iterator	itUser;
	User *curUser;

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

/**
 * пытается найти пользователя с переданным ему именем(пожелаем ему удачи!)
 * @param userName имя предполагаемого пользователя, котрого нужно найти
 * @return указатель на пользователя с необходимым именем или nullptr
 */
User *Server::findUserByName(std::string userName) { //перейдет в класс команды
	std::vector<User *>::iterator it; // итератор по юзерам
	for(it = this->users.begin(); it != this->users.end(); it++){
		User *curUser = *it;
		if (curUser->getNickName() == userName){
			return *it;
		}
	}
	return nullptr;
}

/**
 * пытается найти команду в векторе команд по ее имени
 * @param commandName имя команды
 * @return указатель на класс нужной команды или nullptr
 */
/*Command *Server::findCommandByName(std::string commandName) {
	std::vector<Command *>::iterator it; //итератор по вектору команд
	for(it = this->commands.begin(); it != this->commands.end(); it++){
		Command *curCommand = *it;
		if (curCommand->getName() == commandName){
			return *it;
		}
	}
	return nullptr;
}*/
/**
* разделяет строку по пробелам и возвращает полученный массив, заранее отделяя агрумент после ':'
* @param argString строка-сообщение
* @return вектор аргументов
*/
std::vector<std::string> Server::setArgs(std::string argString) {
	std::vector<std::string> args;
	std::string lastArg;
	size_t pos = 0;
	size_t newPos;

	newPos = argString.find(':', 0);
	if (newPos != std::string::npos){
		lastArg = argString.substr(newPos);
		argString.erase(newPos);
	}
	int i;
	for (i = 0; i < 6; i++){
		newPos = argString.find(' ', pos);
		if (newPos == std::string::npos)
		{
			args.push_back(argString.substr(pos, newPos - pos));
			break;
		}
		args.push_back(argString.substr(pos, newPos - pos));
		pos = newPos + 1;
	}
	if (!lastArg.empty())
		args[i] = lastArg;
	return args;

}

/**
 * пытается найти среди вектора гостей гостя с совпадающим фдшником
 * @param fd необходимый фд
 * @return возвращает указатель на гостя или nullptr, если не нашел
 */
Guest *Server::findGuestByFd(int fd) {
	for (int i = 0; i < this->guests.size(); i++) {
		if (fd == this->guests[i]->getSocketFd()){
			return this->guests[i];
		}
	}
	return nullptr;
}

/**
 * пытается найти среди вектора пользователей пользователя с совпадающим фдшником
 * @param fd необходимый фд
 * @return возвращает указатель на пользователя или nullptr, если не нашел
 */
User *Server::findUserByFd(int fd) {
	for (int i = 0; i < this->users.size(); i++){
		if (fd == this->users[i]->getSocketFd()){
			return this->users[i];
		}
	}
	return nullptr;
}

/**
 * находит какую команду вызывает Юзер и выполняет ее
 * @param user указатель на юзера, который отправил сообщение
 */
void Server::programProcess(User *user) {
	std::vector<std::string> args = setArgs(user->getMessage());
	if (args[0] == "USER"){}
	else if (args[0] == "PASS"){
		this->passCommand(&args,user);
	}
	else if (args[0] == "NICK"){}
	else if (args[0] == "PRIVMSG"){}
	else if (args[0] == "OPER"){}
	else if (args[0] == "QUIT"){}
	else if (args[0] == "JOIN"){}
	else if (args[0] == "PART"){}
	else if (args[0] == "MODE"){}
	else if (args[0] == "NAMES"){}
	else if (args[0] == "LIST"){}
	else if (args[0] == "KICK"){}
	else if (args[0] == "ADMIN"){}
	else if (args[0] == "NOTICE"){}
//	else if (args[0] == ""){}
//	else if (args[0] == ""){}
//	args[0] - имя комманды, которое надо будет найти в мапе
}

/**
 * находит какую команду вызывает гость и выполняет ее
 * @param guest указатель на гостя, который отправил сообщение
 */
void Server::programProcess(Guest *guest) {
	std::vector<std::string> args = setArgs(guest->getMessage());
	//	args[0] - имя комманды, которое надо будет найти в мапе
}

void Server::passCommand(std::vector<std::string> *args, User *user) {
	if (args->size() != 2){
		//todo: error args
	} else{
		user->setPassword(args->at(1));
	}
}

