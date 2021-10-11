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
	virtual ~Server();

	[[noreturn]] void start();
	void init();

	void acceptProcess();
	std::vector<std::string> setArgs(std::string argString);
	void commandProcess(User *user);
	std::string recvMessage(int fd);

	User *findUserByName(std::string userName);
	User *findUserByFd(int fd);
	Channel *findChannelByName(std::string channelName);

	void passCommand(std::vector<std::string> *args, User *user);
	void userCommand(std::vector<std::string> *args, User *user);
	void nickCommand(std::vector<std::string> *args, User *user);
	void operCommand(std::vector<std::string> *args, User *user);
	void privmsgCommand(std::vector<std::string> *args, User *user);
	void joinCommand(std::vector<std::string> *args, User *user);
	void namesCommand(std::vector<std::string> *args, User *user);

	void createChannel(User *user, std::string name);
	void showUsers();
};
#endif //FT_IRC_SERVER_HPP
