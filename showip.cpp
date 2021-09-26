//
// Created by shaurmyashka on 9/23/21.
//

/*
** showip.c — показывает IP-адрес хоста, указанного в командной строке.
*/
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];
	if (argc != 2) {
		fprintf(stderr,"usage: showip hostnamen");
		return 1;
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET или AF_INET6 для указания версии протокола
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(argv[1], nullptr, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %sn", gai_strerror(status));
		return 2;
	}

	printf("IP addresses for %s:nn", argv[1]);

	for(p = res;p != nullptr; p = p->ai_next) {
		void *addr;
		char *ipver;

		// получаем указатель на адрес, по разному в разных протоколах
		if (p->ai_family == AF_INET) { // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} else { // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		// преобразуем IP в строку и выводим его:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("  %s: %sn", ipver, ipstr);
	}

freeaddrinfo(res); // free the linked list

return 0;
}
