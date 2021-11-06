//
// Created by shaurmyashka on 10/24/21.
//

#include <cstring>
#include "Server.hpp"
#include "User.hpp"
#include "Channel.hpp"

/**
 * функция для получения сообщения
 * @param fd фдшник, с которого мы получаем сообщение
 */
std::string Server::recvMessage(int fd) {
	char message[512];
	ssize_t recvByte;
	memset(message, '\0', sizeof(message));
	recvByte = recv(fd, message, sizeof(message), 0);
	if (recvByte <= 0) {
		throw std::runtime_error("recv < 0");
	}
	std::cout << "💬 ➡ " << message << " ⬅ 🐢" << std::endl;
	return (message);
}

/**
 * пытается найти пользователя с переданным ему именем(пожелаем ему удачи!)
 * @param userName имя предполагаемого пользователя, которого нужно найти
 * @return указатель на пользователя с необходимым именем или nullptr
 */
User *Server::findUserByName(const std::string &userName) const {
	for (unsigned int i = 0; i < this->users.size(); i++) {
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
	if (newPos != std::string::npos) {
		argString = argString.substr(0, newPos);
	} else {
		newPos = argString.find('\n');
		if (newPos != std::string::npos) {
			argString = argString.substr(0, newPos);
		}
	}
	if (argString.empty()) {
		return args;
	}
	unsigned long spaceSkip = argString.length() - 1;
	while (argString[spaceSkip] == ' ' && spaceSkip != 0) {
		spaceSkip--;
	}
	if (spaceSkip != 0) {
		argString = argString.substr(0, spaceSkip + 1);
	}
	newPos = argString.find(':', 0);
	if (newPos != std::string::npos) {
		lastArg = argString.substr(newPos + 1);
		argString.erase(newPos);
	}
	int i;
	for (i = 0;; i++) {
		newPos = argString.find(' ', pos);
		if (newPos == std::string::npos) {
			args.push_back(argString.substr(pos, newPos - pos));
			break;
		}
		args.push_back(argString.substr(pos, newPos - pos));
		while (argString[newPos + 1] && argString[newPos + 1] == ' ') {
			newPos++;
		}
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
Channel *Server::findChannelByName(const std::string& channelName) {
	for (unsigned int i = 0; i < this->channels.size(); i++) {
		if (channelName == this->channels[i]->getChannelName()) {
			return this->channels[i];
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
	for (unsigned int i = 0; i < this->users.size(); i++) {
		if (fd == this->users[i]->getSocketFd()) {
			return this->users[i];
		}
	}
	return nullptr;
}

/**
 * создает канал, добавляет его в вектор каналов и добавляет создавшего юзера в этот канал,
 * а так же добавляет канал юзеру в вектор и устанавливает его оператором
 * @param user пользователь создавший канал
 * @param name имя канала
 */
Channel *Server::createChannel(User *user, std::string name) {
	Channel *channel = new Channel(name);
	channels.push_back(channel);
	user->addChannel(channel);
	channel->setUser(user);
	channel->setOperators(user);
	return channel;
}

/**
 * удаляет юзера из сервера (покидает все каналы)
 * @param user указатель на юзера
 */
void Server::removeUser(User *user) {
	for (unsigned int i = 0; i != this->users.size(); i++) {
		if (user == users[i]) {
			users[i]->leaveAllChannels();
			users.erase(users.begin() + i);
			break;
		}
	}
}

/**
 * удаляет структуру pollfd из вектора слушаемых фдшников
 * @param fd фд
 */
void Server::removePollfd(int fd) {
	for (unsigned int i = 0; i < fds.size(); i++) {
		if (fd == fds[i].fd) {
			this->fds.erase(fds.begin() + i);
			break;
		}
	}
}

/**
 * удаляет канал из вектора каналов сервера
 * @param channelName имя канала
 */
void Server::removeChannel(std::string channelName) {
	for (unsigned int i = 0; i < this->channels.size(); i++) {
		if (channelName == this->channels[i]->getChannelName()) {
			this->channels.erase(channels.begin() + i);
			break;
		}
	}
}
