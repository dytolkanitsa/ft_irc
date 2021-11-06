//
// Created by shaurmyashka on 10/24/21.
//

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include "Bot.hpp"
//#include "User.hpp"

Bot::Bot(int socketFd, std::string host, std::string port, std::string pass) : User(socketFd), host(host), port(port), pass(pass) {
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
		{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void Bot::startBot() {
	int sockfd, numbytes;
	char buf[512];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(this->host.c_str(), this->port.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != nullptr; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == nullptr) {
		fprintf(stderr, "client: failed to connect\n");
		exit(1);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	this->setSocketFd(sockfd);
	this->doRegister();
	while(true) {
		if ((numbytes = recv(sockfd, buf, 512 - 1, 0)) == -1) {
			perror("recv");
			exit(1);
		} else {
			buf[numbytes] = '\0';
			printf("client: received %s\n",buf);
			this->commandProcess(buf);
		}


	}


	close(sockfd);

}

void Bot::doRegister() {
	std::string passMessage = "PASS " + this->pass + "\r\n";
	send(this->socketFd, passMessage.c_str(), 8, 0);
	sleep(1);
	send(this->socketFd, "NICK DaBot\r\n", 12, 0);
	sleep(1);
	send(this->socketFd,"USER 1 1 1 1\r\n", 14,0);
}

std::string getMessage(std::string arg){
	std::string message;
	int pos = arg.find("\r\n");
	if (pos != std::string::npos) {
		arg = arg.substr(0, pos);
	}
	pos = arg.find(':',1);
	message = arg.substr(pos + 1);
	unsigned long spaceSkip = message.length() - 1;
	while (message[spaceSkip] == ' ' && spaceSkip != 0) {
		spaceSkip--;
	}
	if (spaceSkip != 0) {
		message = message.substr(0, spaceSkip + 1);
	}
	pos = message.find(':', 0);
	if (pos != std::string::npos) {
		message = message.substr(pos + 1);
		message.erase(pos);
	}
	return message;
}

std::string getNick(std::string arg){
	int pos = arg.find(' ');
	std::string nick = arg.substr(1, pos - 1);
	return nick;
}

void Bot::commandProcess(std::string arg) {
	std::string message = getMessage(arg);
	std::string messageToUser;
	if (message == "da" || message == "Da" || message == "Да" || message == "да"){
		messageToUser = "PRIVMSG " + getNick(arg) + " :П**да\r\n";
		send(this->socketFd, messageToUser.c_str(), messageToUser.length(), 0);
	} else if (message == "нет" || message == "Нет" || message == "Net" || message == "net"){
		messageToUser = "PRIVMSG " + getNick(arg) + " :П*д*ра ответ\r\n";
		send(this->socketFd, messageToUser.c_str(), messageToUser.length(), 0);
	}else if (message == "a" || message == "A" || message == "а" || message == "А"){
		messageToUser = "PRIVMSG " + getNick(arg) + " :Х*й на\r\n";
		send(this->socketFd, messageToUser.c_str(), messageToUser.length(), 0);
	}else if (message == "Ало" || message == "ало" || message == "alo" || message == "Alo"){
		messageToUser = "PRIVMSG " + getNick(arg) + " :х**м по лбу не дало?\r\n";
		send(this->socketFd, messageToUser.c_str(), messageToUser.length(), 0);
	}
}
