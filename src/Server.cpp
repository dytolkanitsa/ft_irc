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
		exit(1);
		// todo: bind error
	}
	freeaddrinfo(serverInfo); // освобождаем связанный список
	this->socketFd = socketFd;
}

/**
 * listen сокета, создание pollfd структуры для этого сокета,
 * добавление в вектор структур, и главный цикл
 */
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
	pollfd nowPollfd;

	for (int i = 0; i < this->fds.size(); i++) {
		nowPollfd = this->fds[i];

		if ((nowPollfd.revents & POLLIN) == POLLIN){ ///модно считать данные

			if (nowPollfd.fd == this->socketFd) { ///accept
				int clientSocket;
				sockaddr_in		clientAddr;
				memset( &clientAddr, 0, sizeof(clientAddr));
				socklen_t socketLen = sizeof (clientAddr);
				clientSocket = accept(this->socketFd, (sockaddr *) &clientAddr, &socketLen);
				if (clientSocket == -1) {
					continue;
				}
				pollfd clientPollfd {clientSocket, POLLIN, 0};
				this->fds.push_back(clientPollfd);
				if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == -1) {
					throw std::runtime_error("fcntl error");
				}
				std::cout<< "cs:" << clientSocket << std::endl;
				std::cout<< "sfd:" << this->socketFd << std::endl;
				User *user = new User(clientSocket, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				this->users.push_back(user);
			}
			else { ///нужно принять данные не с основного сокета, который мы слушаем(клиентского?)
				try {
					std::cout << "fd: " << nowPollfd.fd << std::endl;
					User * curUser = findUserByFd(nowPollfd.fd);
					curUser->setMessage(recvMessage(curUser->getSocketFd()));
					this->commandProcess(curUser);
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
	recvByte = recv(fd, message, sizeof(message), 0);
	if (recvByte <= 0){
		throw std::runtime_error("recv < 0");
	}
	std::cout << "💬 ➡ " << message << " ⬅ 🐢" << std::endl;
	return (message);
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
 * пытется найти канал с нужным именем
 * @param channelName имя канала, который нужно найти
 * @return указатель на канал или nullptr при неудаче
 */
Channel *Server::findChannelByName(std::string channelName) {
	for (int i = 0; i < this->channels.size(); i++){
		if (channelName == this->channels[i]->getChannelName()){
			return this->channels[i];
		}
	}
	return  nullptr;
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
void Server::commandProcess(User *user) {
	std::vector<std::string> args = setArgs(user->getMessage());
	if (args[0] == "USER"){
		this->userCommand(&args, user);
	}
	else if (args[0] == "PASS"){
		this->passCommand(&args,user);
	}
	else if (args[0] == "NICK"){
		this->nickCommand(&args, user);
	}
	else if (args[0] == "PRIVMSG"){
		this->privmsgCommand(&args, user);
	}
	else if (args[0] == "OPER"){
		this->operCommand(&args, user);
	}
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


void Server::passCommand(std::vector<std::string> *args, User *user) {
	if (args->size() != 2){
		//todo: error args
	} else{
		user->setPassword(args->at(1));
	}
}

void Server::userCommand(std::vector<std::string> *args, User *user) {
	if (args->size() != 5){
		throw std::runtime_error("Wrong count of args: PASS <password>");
		//todo: error args
	} else{
		user->setRealName(args->at(4));
//		user->setUsername(args->at(2)); //todo: username?
	}
}

void Server::nickCommand(std::vector<std::string> *args, User *user) {
	if (args->size() != 2){
		throw std::runtime_error("Wrong count of args: NICK <nickname>");
		//todo:: error args
	} else {
		user->setNickName(args->at(1));
	}
}

void Server::operCommand(std::vector<std::string> *args, User *user) {
	if (args->size() != 3){
		throw std::runtime_error("Wrong count of args: OPER <user> <password>");
	} else{
		if (user->getNickName() == args->at(1) && user->getPassword() == args->at(2)){
			user->makeOperator();
		} else{
			throw std::runtime_error("Wrong user or password");
		}
	}
}

/**
 * разбивает сроку по запятым
 * @param receivers срока, которую нужно разбить
 * @return
 */
std::vector<std::string> getReceivers(const std::string& receivers){ //todo: не тестировала
	std::vector<std::string> result;
	size_t pos = 0;
	size_t newPos;

	for (int i = 0; newPos != std::string::npos; i++){
		newPos = receivers.find(',', pos);
		if (newPos == std::string::npos)
			result.push_back(receivers.substr(pos));
		else
			result.push_back(receivers.substr(pos, newPos - pos));
		pos = newPos + 1;
	}
	return result;
}

void Server::privmsgCommand(std::vector<std::string> *args, User *user) {
	unsigned long size = args->size();
	if (size != 2){
		throw std::runtime_error("Wrong count of args: PRIVMSG <receiver>{,<receiver>} <text to be sent>");
	} else {
		std::vector<std::string> receivers = getReceivers(args->at(1));
		for (int i = 1; i < receivers.size(); i++){
			User *recipientUser = this->findUserByName(receivers.at(i));
			if (recipientUser != nullptr){
				recipientUser->messageToUser(args->at(args->size() -1));
			} else{
				Channel *channel = this->findChannelByName(receivers.at(i));
				if (channel == nullptr){
					throw std::runtime_error("Wrong receiver");
				}
				channel->sendMessageToChannel(args->at(args->size() -1), user);
			}
		}
	}
}

void Server::joinCommand(std::vector<std::string> *args, User *user) {
	if (args->size() != 2){
		throw std::runtime_error("Wrong count of args: JOIN <channel>{,<channel>}");
	}
	std::vector<std::string> channelsForJoin = getReceivers(args->at(1));
	for (int i = 1; i < channelsForJoin.size(); i++){
		Channel *channel = findChannelByName(channelsForJoin[i]);
		if (channel == nullptr){
			createChannel(user, channelsForJoin[i]);
		} else {
			channel->sendMessageToChannel(args->at(args->size() - 1), user);
		}
	}
}

void Server::namesCommand(std::vector<std::string> *args, User *user) {
	if (args->size() != 1){ //todo: 1?
		throw std::runtime_error("Wrong count of args: NAMES [<channel>{,<channel>}]");
	}
	if (args->size() > 1){
		std::vector<std::string> namesChannels = getReceivers(args->at(1));
		for (int i = 1; i < namesChannels.size(); i++){
			Channel *channel = findChannelByName(namesChannels[i]);
			if (channel != nullptr){
				//вывести пльзователей
			}
		}
	}
}

/**
 * создает канал, добавляет его в вектор каналов и добавляет создавшего юзера в этот канал
 * @param user пользователь создавший канал
 * @param name имя канала
 */
void Server::createChannel(User *user, std::string name) {
	Channel *channel = new Channel(name);
	channels.push_back(channel);
	channel->setUser(user);// todo: сделат оператором
}

void Server::showUsers() {

}

//throw std::runtime_error("Wrong count of args: ");
