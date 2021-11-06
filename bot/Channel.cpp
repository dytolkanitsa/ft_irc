#include "Channel.hpp"
#include "User.hpp"


Channel::Channel(std::string chName) : channelName(chName) {

}

Channel::~Channel() {
}

std::string Channel::getChannelName(void) {
	return channelName;
}

void Channel::setOperators(User *operatorr) {
	this->userOperator = operatorr;
}

User *Channel::isOperator(User *user) {
	if (userOperator == user)
		return this->userOperator;
	else
		return nullptr;
}

void Channel::setUser(User *user) {
	users.push_back(user);
}

void Channel::sendMessageToChannel(std::string msg, User *sender) {
	std::vector<User *>::const_iterator i;
	for (i = users.begin(); i != users.end(); i++) {
		if (*i != sender)
			(*i)->sendMessage(msg);
	}
}

bool Channel::ifUserExist(std::string userName) {
	for (unsigned int i = 0; i < this->users.size(); i++) {
		if (users[i]->getNickName() == userName) {
			return true;
		}
	}
	return false;
}

void Channel::removeUser(std::string userName) {
	int found = 0;
	for (std::vector<User *>::const_iterator i = users.begin(); i != users.end(); i++) {
		if ((*i)->getNickName() == userName)
			break;
		found++;
	}
	this->users.erase(users.begin() + found);
}

bool Channel::isEmpty() {
	if (users.empty()) {
		return true;
	}
	return false;
}

void Channel::setTopic(std::string newTopic) {
	this->topic = newTopic;
}

std::string Channel::getTopic() {
	return this->topic;
}

std::string Channel::setGetTopic(std::string newTopic) {
	this->setTopic(newTopic);
	return this->getTopic();
}
