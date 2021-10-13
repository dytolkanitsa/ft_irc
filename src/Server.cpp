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
				User * user = new User(clientSocket);
				this->users.push_back(user);
			}
			else { ///нужно принять данные не с основного сокета, который мы слушаем(клиентского?)
				try {
					std::cout << "fd: " << nowPollfd.fd << std::endl;
					User * curUser = findUserByFd(nowPollfd.fd);
					this->commandProcess(*curUser, recvMessage(curUser->getSocketFd()));
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
User * Server::findUserByName(const std::string & userName) const { //перейдет в класс команды
	std::vector<User *>::iterator it; // итератор по юзерам
	for(int i = 0; i < this->users.size(); i++) {
		if (users[i]->getNickName() == userName) {
			return users[i];
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
	size_t newPos = 0;

	newPos = argString.find("\r\n");
	if (newPos != std::string::npos){
		argString = argString.substr(0, newPos);
	}

	newPos = argString.find(':', 0);
	if (newPos != std::string::npos){
		lastArg = argString.substr(newPos + 1);
		argString.erase(newPos);
	}
	int i;
	for (i = 0; i < 6; i++){ //todo: 6?
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
void Server::commandProcess(User & user, const std::string & message) {
	std::vector<std::string> args = setArgs(message);
	try {
		std::string command = args[0];
		args.erase(args.begin());
		if (command == "PASS") {
			this->passCommand(args, user);
		}
		else if (command == "USER") {
			this->userCommand(args, user);
		}
		else if (command == "NICK") {
			this->nickCommand(args, user);
		}
		else if (command == "PRIVMSG") {
			this->privmsgCommand(args, user);
		}
		else if (command == "QUIT"){}
		else if (args[0] == "JOIN"){
			this->joinCommand(args, user);
		}
		else if (args[0] == "PART"){}
		// else if (args[0] == "MODE"){}
		else if (args[0] == "NAMES"){}
		else if (args[0] == "LIST"){}
		else if (args[0] == "KICK"){}
		// else if (args[0] == "ADMIN"){}
		else if (args[0] == "NOTICE"){}
		else if (args[0] == "AWAY"){}
	} catch (std::runtime_error & error) {
		user.messageToUser(error.what());
	}
}

void Server::passCommand(std::vector<std::string> & args, User & user) const {
	if (user.getEnterPassword())
		throw alreadyRegistered(user.getNickName());
	if (args.empty())
		throw needMoreParams(user.getNickName(), "PASS");
	if (args[0] != this->password) {
		throw passMismatch(user.getNickName());
	}
	user.setEnterPassword(true);
}

void Server::userCommand(std::vector<std::string> & args, User & user) const {
	if (args.size() != 4) {
		throw needMoreParams(user.getNickName(), "USER");
	}
	user.setRealName(args[3]);
	user.setRegistered(true);
}

void Server::nickCommand(std::vector<std::string> & args, User & user) const {
	std::string prevNick = user.getNickName();
	if (args.empty()) {
		throw needMoreParams(user.getNickName(), "NICK");
	}
	if (findUserByName(args[0])) {
		throw nickInUse(user.getNickName(), args[0]);
	}
	user.setNickName(args[0]);
	user.messageToUser(":" + prevNick + " NICK " + user.getNickName() + "\r\n");
}

/**
 * разбивает сроку по запятым
 * @param receivers срока, которую нужно разбить
 * @return
 */
std::vector<std::string> getReceivers(const std::string& receivers){ //todo: не тестировала
	std::vector<std::string> result;
	size_t pos = 0;
	size_t newPos = 0;

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

/*
пересылает сообщения в канал или юзеру, если у юзера стоит away message, то юзер автоматически
получает его в качестве ответа
*/
void Server::privmsgCommand(std::vector<std::string> & args, User & user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() != 3) {
		throw needMoreParams(user.getNickName(), "PRIVMSG");
	}
	else {
		std::vector<std::string> receivers = getReceivers(args[1]);
		for (int i = 0; i < receivers.size(); i++) {
			User *recipientUser = this->findUserByName(receivers.at(i));
			if (recipientUser != nullptr){
				recipientUser->messageToUser(args[args.size() -1]);
			if (recipientUser->getAwayMessage().size()!= 0) { //away message
				throw std::runtime_error(recipientUser->getAwayMessage()); // выкидываем юзеру away message другого юзера
			}
			} else{
				Channel *channel = this->findChannelByName(receivers.at(i));
				if (channel == nullptr){
					throw std::runtime_error("Wrong receiver");
				}
				channel->sendMessageToChannel(args.at(args.size() -1), &user);
			}
		}
	}
}

void	Server::noticeCommand(std::vector<std::string> & args, User & user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() != 3) {
		throw needMoreParams(user.getNickName(), "NOTICE");
	}
	else {
		std::vector<std::string> receivers = getReceivers(args[1]);
		for (int i = 0; i < receivers.size(); i++) {
			User *recipientUser = this->findUserByName(receivers.at(i));
			if (recipientUser != nullptr){
				recipientUser->messageToUser(args[args.size() -1]);
			} else{
				Channel *channel = this->findChannelByName(receivers.at(i));
				if (channel == nullptr){
					throw std::runtime_error("Wrong receiver");
				}
				channel->sendMessageToChannel(args.at(args.size() -1), &user);
			}
		}
	}
}

void Server::joinCommand(std::vector<std::string> & args, User & user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() != 2){
		throw needMoreParams(user.getNickName(), "JOIN");
	}
	std::vector<std::string> channelsForJoin = getReceivers(args.at(1));
	for (int i = 1; i < channelsForJoin.size(); i++){
		Channel *channel = findChannelByName(channelsForJoin[i]);
		if (channel == nullptr){
			createChannel(&user, channelsForJoin[i]);
		} else {
			user.addChannel(channel);
			//todo: message about join
//			channel->sendMessageToChannel(args.at(args.size() - 1), &user);
		}
	}
}

void Server::namesCommand(std::vector<std::string> & args, User & user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() != 1) { //todo: 1!?
		throw needMoreParams(user.getNickName(), "NAMES");
	}
	if (args.size() > 1) {
		std::vector<std::string> namesChannels = getReceivers(args.at(1));
		for (int i = 1; i < namesChannels.size(); i++ ){
			Channel *channel = findChannelByName(namesChannels[i]);
			if (channel != nullptr) {
				//todo: вывести пльзователей
			}
		}
	}
}

/*
выводит список каналов на сервере (и их топиков, но мы это опустим)
*/
void	Server::listCommand(std::vector<std::string> & args, User & user)
{
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	if (args.empty()) {
		throw needMoreParams(user.getNickName(), "LIST");
	}
	std::vector<Channel *> channels_ =  this->getChannels();
	for (std::vector<Channel *>::const_iterator i = channels_.begin(); i != channels_.end(); i++)
	{
		user.messageToUser((*i)->getChannelName())
	}
	user.messageToUser("End of LIST\r\n"); // 323* :End of LIST ???
}

/*
!НЕ СДЕЛАН
кикает пользователя с канала
*/
void	Server::kickCommand(std::vector<std::string> & args, User & user)
{
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
}

/*
функция устанавливапет, что у юзера есть эвэй сообщение, которое будет автоматически
отсылаться любому другому юзеру, который ему напишет, в этой функции мы только его устанавливаем
и когда установили, пишем, что You have been marked as being away, реализация этой системы в 
привмсг
*/
void Server::awayCommand(std::vector<std::string> & args, User & user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	else {
		if (args.size() == 1) {
			std::string awayMessage = args[0]; // там же текст
			user.setAwayMessage(awayMessage);
			user.messageToUser(":You have been marked as being away") // 306 ошибка
		}
		else
			throw needMoreParams(user.getNickName(), "AWAY");
}
/**
 * создает канал, добавляет его в вектор каналов и добавляет создавшего юзера в этот канал, а так же добавляет канал юзеру в вектор
 * @param user пользователь создавший канал
 * @param name имя канала
 */
void Server::createChannel(User *user, std::string name) {
	Channel *channel = new Channel(name);
	channels.push_back(channel);
	user->addChannel(channel);
	channel->setUser(user);// todo: сделат оператором
}

void Server::showUsers() {

}

std::vector<Channel *> Server::getChannels()
{
	return channels;
}

std::string Server::constructError(const std::string & code,
								   const std::string & message,
								   const std::string & nick = "*",
								   const std::string & secondParam = "") const {
	return code + " " + nick + " " + secondParam + " " + ":" + message + "\r\n";
}

std::runtime_error Server::alreadyRegistered(const std::string & nick) const {
	return std::runtime_error(constructError("462", "Not enough parameters", nick));
}

std::runtime_error Server::needMoreParams(const std::string & nick, const std::string & command) const {
	return std::runtime_error(constructError("461", "Not enough parameters", nick));
}

std::runtime_error Server::passMismatch(const std::string & nick) const {
	return std::runtime_error(constructError("464", "Password incorrect", nick));
}

std::runtime_error Server::nickInUse(const std::string & nick, const std::string & newNick) const {
	return std::runtime_error(constructError("433", "Nickname is already in use", nick, newNick));
}

std::runtime_error Server::connectionRestricted(const std::string &nick) const {
	return std::runtime_error(constructError("484", "Your connection is restricted!", nick));
}
