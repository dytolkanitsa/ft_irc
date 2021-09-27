//
// Created by OUT-Podovalov-PG on 26.09.2021.
//

#ifndef FT_IRC_SERVER_HPP
#define FT_IRC_SERVER_HPP

#include <string>
#include <sys/poll.h>
#include <vector>

class Server {
private:
	int socketFd;
	const std::string * host;
	const std::string & port;
	const std::string & password;
	std::vector<pollfd> fds;
public:
	Server(const std::string * host, const std::string & port, const std::string & password);
	void init();
	void start();
//	void stop();
	virtual ~Server();

};

#endif //FT_IRC_SERVER_HPP
