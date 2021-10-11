//
// Created by OUT-Podovalov-PG on 26.09.2021.
//

#ifndef FT_IRC_SERVER_HPP
#define FT_IRC_SERVER_HPP

#include <string>
#include <sys/poll.h>
#include <vector>
#include "User.hpp"
#include "Channel.hpp"



class Server {
private:
	int socketFd;
	const std::string * host;
	const std::string & port;
	const std::string & password;
	std::vector<pollfd> fds;
	std::vector<User *> users;
	std::vector<Channel *> channels;
public:

	Server(const std::string * host, const std::string & port, const std::string & password);
	void init();

	[[noreturn]] void start();
//	void stop();
	virtual ~Server();
	void acceptProcess();
	std::string recvMessage(int fd);
	void sendMessage(User * user);
	User *findUserByName(const std::string & userName) const;
	User *findUserByFd(int fd);
	Channel *findChannelByName(std::string channelName);
	void commandProcess(User & user, const std::string & message);
//	Command *findCommandByName(std::string commandName); нужна map
	std::vector<std::string> setArgs(std::string argString);

	// Errors
	std::string constructError(const std::string & code,
							   const std::string & message,
							   const std::string & nick,
							   const std::string & secondParam) const;
	std::runtime_error alreadyRegistered(const std::string & nick) const;
	std::runtime_error needMoreParams(const std::string & nick, const std::string & command) const;
	std::runtime_error passMismatch(const std::string & nick) const;
	std::runtime_error nickInUse(const std::string &nick, const std::string &newNick) const;
	std::runtime_error connectionRestricted(const std::string &nick) const;

	// Commands
	void passCommand(std::vector<std::string> & args, User & user) const;
	void userCommand(std::vector<std::string> & args, User & user) const;
	void nickCommand(std::vector<std::string> & args, User & user) const;
	void privmsgCommand(std::vector<std::string> & args, User & user);
	void joinCommand(std::vector<std::string> & args, User & user);
	void namesCommand(std::vector<std::string> & args, User & user);


	void createChannel(User *user, std::string name);
	void showUsers();




};
#endif //FT_IRC_SERVER_HPP
