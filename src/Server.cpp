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
	if (rp == nullptr)  {
		throw std::runtime_error("bind error");
		exit(1);
	}
	freeaddrinfo(serverInfo); // освобождаем связанный список
	this->socketFd = socketFd;
}

/**
 * listen сокета, создание pollfd структуры для этого сокета,
 * добавление в вектор структур, и главный цикл
 */
[[noreturn]] void Server::start() {
	if (listen(this->socketFd, 10) == -1){
		throw std::runtime_error("listen error");
	}
	pollfd sPollfd {this->socketFd, POLLIN, 0};
	if (fcntl(this->socketFd, F_SETFL, O_NONBLOCK) == -1){
		throw std::runtime_error("fcntl error");
	}
	this->fds.push_back(sPollfd);
	std::vector<pollfd>::iterator	it;
	while(true){
		it = this->fds.begin();
		if (poll(&(*it), this->fds.size(), -1) == -1){
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
		if ((nowPollfd.revents & POLLHUP) == POLLHUP){ ///кто-то оборвал соединение
			User *user = findUserByFd(nowPollfd.fd);
			if (user == nullptr){
				continue; // я вот вообще честно говоря вообще не знаю, когда такое может произойти
			}
			this->removeUser(user);
			close(fds[i].fd);
			this->fds.erase(fds.begin() + i);
		}
	}
}

/**
 * функция для получения сообщения и добавления к нему "\r\n"
 * @param user класс пользователя для получения сообщения
 */
std::string Server::recvMessage(int fd) {
	char message[512];
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
	size_t newPos;

	newPos = argString.find("\r\n");
	if (newPos != std::string::npos){
		argString = argString.substr(0, newPos);
	} else{
		newPos = argString.find('\n');
		if (newPos != std::string::npos){
			argString = argString.substr(0, newPos);
		}
	}
	if (argString.empty()){
		return args;
	}
	unsigned long spaceSkip = argString.length() - 1;
	while(argString[spaceSkip] == ' ' && spaceSkip != 0){
		spaceSkip--;
	}
	if (spaceSkip != 0){
		argString = argString.substr(0, spaceSkip + 1);
	}
	newPos = argString.find(':', 0);
	if (newPos != std::string::npos){
		lastArg = argString.substr(newPos + 1);
		argString.erase(newPos);
	}
	int i;
	for (i = 0;; i++){
		newPos = argString.find(' ', pos);
		if (newPos == std::string::npos)
		{
			args.push_back(argString.substr(pos, newPos - pos));
			break;
		}
		args.push_back(argString.substr(pos, newPos - pos));
		while(argString[newPos + 1] && argString[newPos + 1] == ' '){
			newPos++;
		}
		pos = newPos + 1;
	}
//		args.push_back(argString.substr(pos));
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
	if (args.empty()){
		return;
	}
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
		else if (command == "JOIN"){
			this->joinCommand(args, user);
		}
		else if (command == "LIST"){
			this->listCommand(args, user);
		}
		else if (command == "NOTICE"){
			this->noticeCommand(args, user);
		}
		else if (command == "AWAY"){
			this->awayCommand(args, user);
		}
		else if (command == "QUIT"){
			this->quitCommand(args, user);
		}
		else if (command == "PART"){
			this->partCommand(args, user);
		}
		else if (command == "TOPIC"){
			this->topicCommand(args, user);
		}
		else if (command == "KICK"){
            this->kickCommand(args, user);
        }
	} catch (std::string & error) {
		user.sendMessage(error);
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
	if (user.getRegistered()) {
		throw alreadyRegistered(user.getNickName());
	}
	user.setRealName(args[3]);
	user.setRegistered(true);
	user.sendMessage(welcomeMsg(user.getNickName()));
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
	user.sendMessage(this->constructMessage(prevNick, "NICK", user.getNickName()));
}

/**
 * разбивает сроку по запятым
 * @param receivers срока, которую нужно разбить
 * @return
 */
std::vector<std::string> getReceivers(std::string receivers){
	std::vector<std::string> result;
	size_t pos = 0;
	size_t newPos = 0;
	unsigned long spaceSkip = receivers.length();

	while(receivers[spaceSkip] == ' ' && spaceSkip != 0){
		spaceSkip--;
	}
	if (spaceSkip != 0){
		receivers = receivers.substr(0, spaceSkip + 1);
	}

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
	if (args.size() < 2) {
		throw needMoreParams(user.getNickName(), "PRIVMSG");
	}
	else {
		std::vector<std::string> receivers = getReceivers(args[0]);
		for (int i = 0; i < receivers.size(); i++) {
			User *recipientUser = this->findUserByName(receivers.at(i));
			if (recipientUser != nullptr) {
				recipientUser->sendMessage(constructMessage(user.getNickName(), "PRIVMSG", recipientUser->getNickName(), args[args.size() - 1]));
				if (!recipientUser->getAwayMessage().empty()) { //away message
					// выкидываем юзеру away message другого юзера
//					recipientUser->sendMessage(constructReply(recipientUser->getNickName(), "PRIVMSG", user.getNickName(), recipientUser->getAwayMessage()));
					user.sendMessage(rplAway(user.getNickName(), recipientUser->getNickName(), recipientUser->getAwayMessage()));
				}
			} else {
				Channel *channel = this->findChannelByName(receivers.at(i));
				if (channel == nullptr){
					throw noSuchNick(user.getNickName(), receivers.at(i));
				}
				if (channel->ifUserExist(user.getNickName())) /*проверка на тор, чьо юзер вообще есть на канале*/
					channel->sendMessageToChannel(constructMessage(user.getNickName(), "PRIVMSG", channel->getChannelName(), args.at(args.size() -1)), &user);
				else
					throw notOnChannel(receivers[i], user.getNickName());
			}
		}
	}
}

void	Server::noticeCommand(std::vector<std::string> & args, User & user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() < 2) {
		throw needMoreParams(user.getNickName(), "NOTICE");
	}
	else {
		std::vector<std::string> receivers = getReceivers(args[0]);
		for (int i = 0; i < receivers.size(); i++) {
			User *recipientUser = this->findUserByName(receivers.at(i));
			if (recipientUser != nullptr){
				recipientUser->sendMessage(args[args.size() - 1]);
			} else{
				Channel *channel = this->findChannelByName(receivers.at(i));
				if (channel == nullptr){
					recipientUser->sendMessage(rplAway(user.getNickName(), recipientUser->getNickName(), recipientUser->getAwayMessage()));
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
	if (args.size() != 1){
		throw needMoreParams(user.getNickName(), "JOIN");
	}
	std::vector<std::string> channelsForJoin = getReceivers(args.at(0));
	for (int i = 0; i < channelsForJoin.size(); i++) {
		Channel *channel = findChannelByName(channelsForJoin[i]);
		if (channel == nullptr){
			channel = createChannel(&user, channelsForJoin[i]);
		} else {
			if (!channel->ifUserExist(user.getNickName()))
			{
				user.addChannel(channel);
				channel->setUser(&user);
			}
		}
		user.sendMessage(rplTopic(user.getNickName(), channel->getChannelName(), channel->getTopic()));
		user.sendMessage(this->constructMessage(user.getNickName(), "JOIN", channel->getChannelName()));
		channel->sendMessageToChannel(this->constructMessage(user.getNickName(), "JOIN", channel->getChannelName()), &user);
	}
}

void Server::kickCommand(std::vector<std::string> & args, User & user)
{
    if (!user.getRegistered()) {
        throw connectionRestricted(user.getNickName());
    }
    if (args.size() < 2 || args.size() > 3) { // без комментария или с комментарием
        throw needMoreParams(user.getNickName(), "KICK");
    }
    std::vector<std::string> channelsForKick = getReceivers(args.at(0));
    for (int i = 0; i < channelsForKick.size(); i++) {
        Channel *channel = findChannelByName(channelsForKick[i]);
        if (channel == nullptr)
        {
            user.sendMessage("403 *" + channelsForKick[i] + " :No such channel");
        }
        else {
            if (!channel->getOperator()) {
                user.sendMessage("482 *" + channelsForKick[i] + " :You're not channel operator");
            }
            std::vector<std::string> receivers = getReceivers(args[1]);
            for (int i = 0; i < receivers.size(); i++) {
                User *recipientUser = this->findUserByName(receivers.at(i));
                if (recipientUser == nullptr)
                    recipientUser->sendMessage(args[args.size() - 1]);
                    else {
                        channel->removeUser(recipientUser->getNickName());
                    }
            }
        }
    }
}

/*
выводит список каналов на сервере
*/
void	Server::listCommand(std::vector<std::string> & args, User & user)
{
	if (!user.getRegistered()) {
        throw connectionRestricted(user.getNickName());
        }
    if (args.empty()) {
        throw needMoreParams(user.getNickName(), "LIST");
    }
	std::string channelsList;
    for (int i = 0; i != channels.size(); i++)
    {
    	channelsList += channels[i]->getChannelName() + "\t\t:" + channels[i]->getTopic() + '\n';
    }
    channelsList.erase(channelsList.length() -1);
    user.sendMessage(channelsList);
    user.sendMessage("323* :End of LIST");
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
	}
	else {
		if (args.size() == 1) {
			std::string awayMessage = args[0];
			user.setAwayMessage(awayMessage);
			throw awayMessageHaveBeenSet(user.getNickName());
		}
		else {
            if (args.empty()) {
                user.setAwayMessage("");
                throw awayMessageHaveBeenUnset(user.getNickName());
            }
        }
	}
}

/**
 * создает канал, добавляет его в вектор каналов и добавляет создавшего юзера в этот канал, а так же добавляет канал юзеру в вектор
 * @param user пользователь создавший канал
 * @param name имя канала
 */
Channel * Server::createChannel(User *user, std::string name) {
	Channel *channel = new Channel(name);
	channels.push_back(channel);
	user->addChannel(channel);
	channel->setUser(user);
	channel->setOperators(user);
	return channel;
}

void Server::partCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()){
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() != 1){
		throw needMoreParams(user.getNickName(), "PART");
	}
	std::vector<std::string> receivers = getReceivers(args[0]);
	for(int i = 0; i < receivers.size(); i++){
		Channel *channel = findChannelByName(receivers[i]);
		if (channel == nullptr){
			throw noSuchNick(user.getNickName(), receivers[i]);
		}
		if (channel->ifUserExist(user.getNickName())){
			channel->removeUser(user.getNickName());
			if (channel->isEmpty()){
				removeChannel(channel->getChannelName());
			}
			throw "you leave a channel " + receivers[i];
		}
		else
			throw notOnChannel(receivers[i], user.getNickName());
	}
}

void Server::topicCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()){
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() == 1 || args.size() == 2){
		Channel * channel = this->findChannelByName(args[0]);
		if (channel != nullptr) {
			if (channel->ifUserExist(user.getNickName())) {
				std::string topic = args.size() == 1 ? channel->getTopic() : channel->setGetTopic(args[1]); // если нужно просто получить топик, то мы вызываем просто геттер, но для переутсановки вызывает сеттергеттер
				throw rplTopic(user.getNickName(), channel->getChannelName(), topic); //если сюда придет пустая строка, то он поймет, что топика нет
			}
			else
				throw notOnChannel(user.getNickName(), channel->getChannelName());
		} else
			throw noSuchNick(user.getNickName(), args[0]);
	} else{
		throw needMoreParams(user.getNickName(), "TOPIC");
	}

}

std::vector<Channel *> Server::getChannels()
{
	return channels;
}

std::string Server::constructReply(const std::string & code,
								   const std::string & message,
								   const std::string & nick = "*",
								   const std::string & secondParam = "") const {
	return code + " " + nick + " " + secondParam + " " + ":" + message + "\r\n";
}

std::string Server::constructMessage(const std::string & sender,
									 const std::string & command,
									 const std::string & recipient,
									 const std::string & message) const {
	return ":" + sender + " " + command + " " + recipient + " " + ((message.empty()) ? "" : ":" + message) + "\r\n";
}

std::string Server::alreadyRegistered(const std::string & nick) const {
	return constructReply("462", "You may not reregister", nick);
}

std::string Server::needMoreParams(const std::string & nick, const std::string & command) const {
	return constructReply("461", "Not enough parameters", nick);
}

std::string Server::passMismatch(const std::string & nick) const {
	return constructReply("464", "Password incorrect", nick);
}

std::string Server::nickInUse(const std::string & nick, const std::string & newNick) const {
	return constructReply("433", "Nickname is already in use", nick, newNick);
}

std::string Server::connectionRestricted(const std::string &nick) const {
	return constructReply("484", "Your connection is restricted!", nick);
}

void Server::removeUser(User *user) {
	for(int i = 0; i != this->users.size(); i++){
		if (user == users[i]){
			users[i]->leaveAllChannels();
			users.erase(users.begin() + i);
			break;
		}
	}
}



void Server::quitCommand(std::vector<std::string> &args, User &user) {
	this->removeUser(&user);
	close(user.getSocketFd());
	this->removePollfd(user.getSocketFd());
}

std::string Server::awayMessageHaveBeenSet(const std::string &nick) const {
    return constructReply("306", "You have been marked as being away", nick);
}

std::string Server::noSuchNick(const std::string &nick, const std::string & recipient) const {
	return constructReply("401", "No such nick/channel", nick, recipient);
}

std::string Server::rplAway(const std::string &nick, const std::string &recipient, const std::string &message) const {
	return constructReply("401", message, nick, recipient);
}

std::string Server::welcomeMsg(const std::string &nick) const {
	return constructReply("001", "Welcome to the Internet Relay Network!", nick);
}


std::string Server::awayMessageHaveBeenUnset(const std::string &nick) const {
    return constructReply("306", "You are no longer marked as being away", nick);
}

std::string Server::NoRecipientGiven(const std::string &nick) const {
    return constructReply("411", ":No recipient given ", nick);
}

void Server::removePollfd(int fd) {
//	for (int i = 0; i < this->fds.size(); i++){
//		if (fd == this->fds[i].fd){
    for (int i = 0; i < fds.size(); i++){
        if (fd == fds[i].fd){
			this->fds.erase(fds.begin() + i);
			break;
		}
	}
}

std::string Server::notOnChannel(const std::string &nick, const std::string &channel) const {
	return constructReply("442", ":You're not on that channel ", nick, channel);
}

void Server::removeChannel(std::string channelName) {
	for(int i = 0; i < this->channels.size(); i++){
		if (channelName == this->channels[i]->getChannelName()){
			this->channels.erase(channels.begin() + i);
			break;
		}
	}
}

std::string Server::rplTopic(const std::string &nick, const std::string &channel, const std::string& topic) const {
	if (!topic.empty())
		return constructReply("332", topic, nick, channel);
	else
		return constructReply("331", "No topic is set", nick, channel);
}

