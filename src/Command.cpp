//
// Created by shaurmyashka on 10/24/21.
//

#include <unistd.h>
#include "Server.hpp"
#include "User.hpp"
#include "Channel.hpp"

/**
 * разбивает сроку по запятым
 * @param receivers срока, которую нужно разбить
 * @return
 */
std::vector<std::string> getReceivers(std::string receivers) {
	std::vector<std::string> result;
	size_t pos = 0;
	size_t newPos = 0;
	unsigned long spaceSkip = receivers.length();

	while (receivers[spaceSkip] == ' ' && spaceSkip != 0) {
		spaceSkip--;
	}
	if (spaceSkip != 0) {
		receivers = receivers.substr(0, spaceSkip + 1);
	}

	for (int i = 0; newPos != std::string::npos; i++) {
		newPos = receivers.find(',', pos);
		if (newPos == std::string::npos)
			result.push_back(receivers.substr(pos));
		else
			result.push_back(receivers.substr(pos, newPos - pos));
		pos = newPos + 1;
	}
	return result;
}

void Server::quitCommand(std::vector<std::string> &args, User &user) {
	this->removeUser(&user);
	close(user.getSocketFd());
	this->removePollfd(user.getSocketFd());
}

void Server::topicCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() == 1 || args.size() == 2) {
		Channel *channel = this->findChannelByName(args[0]);
		if (channel != nullptr) {
			if (channel->ifUserExist(user.getNickName())) {
				std::string topic = args.size() == 1 ? channel->getTopic() : channel->setGetTopic(args[1]); // если нужно просто получить топик, то мы вызываем просто геттер, но для переутсановки вызывает сеттергеттер
				throw rplTopic(user.getNickName(), channel->getChannelName(), topic); //если сюда придет пустая строка, то он поймет, что топика нет
			} else
				throw notOnChannel(user.getNickName(), channel->getChannelName());
		} else
			throw noSuchNick(user.getNickName(), args[0]);
	} else {
		throw needMoreParams(user.getNickName(), "TOPIC");
	}

}

void Server::partCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() != 1) {
		throw needMoreParams(user.getNickName(), "PART");
	}
	std::vector<std::string> receivers = getReceivers(args[0]);
	for (int i = 0; i < receivers.size(); i++) {
		Channel *channel = findChannelByName(receivers[i]);
		if (channel == nullptr) {
			throw noSuchNick(user.getNickName(), receivers[i]);
		}
		if (channel->ifUserExist(user.getNickName())) {
			channel->removeUser(user.getNickName());
			if (channel->isEmpty()) {
				removeChannel(channel->getChannelName());
			}
			throw "you leave a channel " + receivers[i];
		} else
			throw notOnChannel(receivers[i], user.getNickName());
	}
}

void Server::awayCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	} else {
		if (args.size() == 1) {
			std::string awayMessage = args[0];
			user.setAwayMessage(awayMessage);
			throw awayMessageHaveBeenSet(user.getNickName());
		} else {
			if (args.empty()) {
				user.setAwayMessage("");
				throw awayMessageHaveBeenUnset(user.getNickName());
			}
		}
	}
}

void Server::listCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	std::string channelsList;
	for (int i = 0; i != channels.size(); i++) {
		channelsList += channels[i]->getChannelName() + "\t\t:" + channels[i]->getTopic() + '\n';
	}
	channelsList.erase(channelsList.length() - 1);
	user.sendMessage(constructReply("322", channelsList, user.getNickName()));
	user.sendMessage(constructReply("323", "End of /LIST", user.getNickName()));
}

void Server::kickCommand(std::vector<std::string> & args, User & user)
{
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() < 2) {
		throw needMoreParams(user.getNickName(), "KICK");
	}
	Channel *channel = findChannelByName(args[0]);
	if (channel == nullptr)
	{
		throw "403 * " + args[0] + " :No such channel";
	}
	else {
		if (!channel->isOperator(&user)) {
			throw "482 * " + args[0] + " :You're not channel operator";
		}
		User *recipientUser = this->findUserByName(args[1]);
		if (recipientUser != nullptr) {
			recipientUser->sendMessage(constructMessage(user.getNickName(), "KICK", recipientUser->getNickName(), args.size() == 2 ? "" : args[2]));
			channel->removeUser(recipientUser->getNickName());
		}
		else {
			throw noSuchNick(args[1], user.getNickName());
		}
	}
}

void Server::joinCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() != 1) {
		throw needMoreParams(user.getNickName(), "JOIN");
	}
	std::vector<std::string> channelsForJoin = getReceivers(args.at(0));
	for (int i = 0; i < channelsForJoin.size(); i++) {
		Channel *channel = findChannelByName(channelsForJoin[i]);
		if (channel == nullptr) {
			channel = createChannel(&user, channelsForJoin[i]);
		} else {
			if (!channel->ifUserExist(user.getNickName())) {
				user.addChannel(channel);
				channel->setUser(&user);
			}
		}
		user.sendMessage(rplTopic(user.getNickName(), channel->getChannelName(), channel->getTopic()));
		user.sendMessage(this->constructMessage(user.getNickName(), "JOIN", channel->getChannelName()));
		channel->sendMessageToChannel(this->constructMessage(user.getNickName(), "JOIN", channel->getChannelName()), &user);
	}
}

void Server::noticeCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() < 2) {
		throw needMoreParams(user.getNickName(), "NOTICE");
	} else {
		std::vector<std::string> receivers = getReceivers(args[0]);
		for (int i = 0; i < receivers.size(); i++) {
			User *recipientUser = this->findUserByName(receivers.at(i));
			if (recipientUser != nullptr) {
				recipientUser->sendMessage(constructMessage(user.getNickName(), "NOTICE", recipientUser->getNickName(), args[args.size() - 1]));
			} else {
				Channel *channel = this->findChannelByName(receivers.at(i));
				if (channel == nullptr) {
					throw noSuchNick(user.getNickName(), receivers.at(i));
				}
				if (channel->ifUserExist(user.getNickName())) /*проверка на тор, чьо юзер вообще есть на канале*/
					channel->sendMessageToChannel(constructMessage(user.getNickName(), "NOTICE", channel->getChannelName(), args.at(args.size() - 1)), &user);
				else
					throw notOnChannel(receivers[i], user.getNickName());
			}
		}
	}
}

void Server::privmsgCommand(std::vector<std::string> &args, User &user) {
	if (!user.getRegistered()) {
		throw connectionRestricted(user.getNickName());
	}
	if (args.size() < 2) {
		throw needMoreParams(user.getNickName(), "PRIVMSG");
	} else {
		std::vector<std::string> receivers = getReceivers(args[0]);
		for (int i = 0; i < receivers.size(); i++) {
			User *recipientUser = this->findUserByName(receivers.at(i));
			if (recipientUser != nullptr) {
				recipientUser->sendMessage(constructMessage(user.getNickName(), "PRIVMSG", recipientUser->getNickName(), args[args.size() - 1]));
				if (!recipientUser->getAwayMessage().empty()) { //away message
					user.sendMessage(rplAway(user.getNickName(), recipientUser->getNickName(), recipientUser->getAwayMessage()));
				}
			} else {
				Channel *channel = this->findChannelByName(receivers.at(i));
				if (channel == nullptr) {
					throw noSuchNick(user.getNickName(), receivers.at(i));
				}
				if (channel->ifUserExist(user.getNickName())) /*проверка на тор, чьо юзер вообще есть на канале*/
					channel->sendMessageToChannel(constructMessage(user.getNickName(), "PRIVMSG", channel->getChannelName(), args.at(args.size() - 1)), &user);
					else
						throw notOnChannel(receivers[i], user.getNickName());
			}
		}
	}
}

void Server::nickCommand(std::vector<std::string> &args, User &user) {
	if (!user.getEnterPassword())
		throw user.getNickName() + "Enter your pass first!";
	std::string prevNick = user.getNickName();
	if (args.empty()) {
		throw needMoreParams(user.getNickName(), "NICK");
	}
	if (findUserByName(args[0])) {
		throw nickInUse(user.getNickName(), args[0]);
	}
	user.setNickName(args[0]);
	user.sendMessage(this->constructMessage(prevNick, "NICK", user.getNickName()));
	user.setEnterNick(true);
}

void Server::userCommand(std::vector<std::string> &args, User &user) {
	if (!user.getEnterNick())
		throw user.getNickName() + "Enter your nick first!";
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

void Server::passCommand(std::vector<std::string> &args, User &user) {
	if (user.getEnterPassword())
		throw alreadyRegistered(user.getNickName());
	if (args.empty())
		throw needMoreParams(user.getNickName(), "PASS");
	if (args[0] != this->password) {
		throw passMismatch(user.getNickName());
	}
	user.setEnterPassword(true);
}
