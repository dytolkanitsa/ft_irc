//
// Created by OUT-Podovalov-PG on 26.09.2021.
//

#ifndef FT_IRC_SERVER_HPP
#define FT_IRC_SERVER_HPP

#include <string>
#include <iostream>
#include <sys/poll.h>
#include <vector>
//#include "User.hpp"
//#include "Channel.hpp"

class User;
class Channel;

class Server {
private:
	int                     socketFd;
	const std::string *     host;
	const std::string &     port;
	const std::string &     password;
	std::vector<pollfd>     fds;
	std::vector<User *>     users;
	std::vector<Channel *>  channels;
public:

	Server(const std::string * host, const std::string & port, const std::string & password);
	virtual ~Server();

	[[noreturn]] void start();
	void init();

	void                     acceptProcess();
	std::string              recvMessage(int fd);

	User                    *findUserByName(const std::string & userName) const;
	User                    *findUserByFd(int fd);
	Channel                 *findChannelByName(std::string channelName);
	void                     commandProcess(User & user, const std::string & message);
//	Command *findCommandByName(std::string commandName); нужна map
	std::vector<std::string> setArgs(std::string argString);

	// Replies
	std::string constructReply(const std::string & code,
							   const std::string & message,
							   const std::string & nick,
							   const std::string & secondParam) const;
	std::string constructMessage(const std::string & sender,
								 const std::string & command,
								 const std::string & recipient,
								 const std::string & message = "") const;
	std::string alreadyRegistered(const std::string & nick) const;
	std::string needMoreParams(const std::string & nick, const std::string & command) const;
	std::string passMismatch(const std::string & nick) const;
	std::string nickInUse(const std::string &nick, const std::string &newNick) const;
	std::string connectionRestricted(const std::string &nick) const;
    std::string awayMessageHaveBeenSet(const std::string &nick) const;
	std::string noSuchNick(const std::string & nick, const std::string & recipient) const;
	std::string rplAway(const std::string & nick, const std::string & recipient, const std::string & message) const;
	std::string welcomeMsg(const std::string & nick) const;
	std::string NoRecipientGiven(const std::string &nick) const;
	std::string awayMessageHaveBeenUnset(const std::string &nick) const;
	std::string notOnChannel(const std::string &nick, const std::string &channel) const;
	std::string rplTopic(const std::string &nick, const std::string &channel, const std::string& topic) const;

	// Commands
	void passCommand(std::vector<std::string> & args, User & user) const;
	void userCommand(std::vector<std::string> & args, User & user) const;
	void nickCommand(std::vector<std::string> & args, User & user) const;
    void privmsgCommand(std::vector<std::string> & args, User & user);
    void noticeCommand(std::vector<std::string> & args, User & user);
    void listCommand(std::vector<std::string> & args, User & user);
    void awayCommand(std::vector<std::string> & args, User & user);
    void joinCommand(std::vector<std::string> & args, User & user);
	void quitCommand(std::vector<std::string> & args, User & user);
	void partCommand(std::vector<std::string> & args, User & user);
	void topicCommand(std:: vector<std::string> & args, User & user);

    void kickCommand(std::vector<std::string> & args, User & user);
	Channel * createChannel(User *user, std::string name);

	std::vector<Channel *> getChannels();
	void removeUser(User *user);
	void removePollfd(int fd);
	void removeChannel(std::string channelName);
};
#endif //FT_IRC_SERVER_HPP
