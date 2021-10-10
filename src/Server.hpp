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
	User *findUserByName(std::string userName);
	User *findUserByFd(int fd);
	Channel *findChannelByName(std::string channelName);
	void commandProcess(User *user);
//	Command *findCommandByName(std::string commandName); нужна map
	std::vector<std::string> setArgs(std::string argString);
	void passCommand(std::vector<std::string> *args, User *user);
	void userCommand(std::vector<std::string> *args, User *user);
	void nickCommand(std::vector<std::string> *args, User *user);
	void operCommand(std::vector<std::string> *args, User *user);
	void privmsgCommand(std::vector<std::string> *args, User *user);



};
#endif //FT_IRC_SERVER_HPP
