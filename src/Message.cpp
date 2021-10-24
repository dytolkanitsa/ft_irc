//
// Created by shaurmyashka on 10/24/21.
//
#include "Server.hpp"

/**
 * собирает сообщение из полученных параметров
 * @param code - код ответа
 * @param message - сообщение ответа
 * @param nick - ник юзера (необязательный)
 * @param secondParam - опциональный параметр команды (необязательный)
 * @return строку для отправки
 */
std::string Server::constructReply(const std::string &code,
								   const std::string &message,
								   const std::string &nick,
								   const std::string &secondParam) const {
	return code + " " + nick + " " + secondParam + " " + ":" + message + "\r\n";
}

/**
 * собирает сообщение из полученных параметров
 * @param sender ник отправителя
 * @param command команда
 * @param recipient ник получателя
 * @param message сообщение для получателя
 * @return строку, которую нужно отправить
 */
std::string Server::constructMessage(const std::string &sender,
									 const std::string &command,
									 const std::string &recipient,
									 const std::string &message) const {
	return ":" + sender + " " + command + " " + recipient + " " + ((message.empty()) ? "" : ":" + message) + "\r\n";
}

std::string Server::alreadyRegistered(const std::string &nick) const {
	return constructReply("462", "You may not reregister", nick);
}

std::string Server::needMoreParams(const std::string &nick, const std::string &command) const {
	return constructReply("461", "Not enough parameters", nick);
}

std::string Server::passMismatch(const std::string &nick) const {
	return constructReply("464", "Password incorrect", nick);
}

std::string Server::nickInUse(const std::string &nick, const std::string &newNick) const {
	return constructReply("433", "Nickname is already in use", nick, newNick);
}

std::string Server::connectionRestricted(const std::string &nick) const {
	return constructReply("484", "Your connection is restricted!", nick);
}

std::string Server::awayMessageHaveBeenSet(const std::string &nick) const {
	return constructReply("306", "You have been marked as being away", nick);
}

std::string Server::noSuchNick(const std::string &nick, const std::string &recipient) const {
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

std::string Server::rplTopic(const std::string &nick, const std::string &channel, const std::string &topic) const {
	if (!topic.empty())
		return constructReply("332", topic, nick, channel);
	else
		return constructReply("331", "No topic is set", nick, channel);
}

std::string Server::notOnChannel(const std::string &nick, const std::string &channel) const {
	return constructReply("442", ":You're not on that channel ", nick, channel);
}
