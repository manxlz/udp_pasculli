/*
 ============================================================================
 Name        : server_udp_pasculli.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdbool.h>
#include "protocol.h"
#include "calculator.h"

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {
#if defined WIN32
	WSADATA wsa_data;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (iResult != 0) {
		errorhandler("Error at WSAStartup()\n");
		return EXIT_FAILURE;
	}
#endif
	operationStruct operation;
	const char *ip;
	int sock = 0;
	int port = 0;
	int cliAddrLen = 0;
	int recvMsgSize = 0;
	boolean b = false;
	float tot = 0;
	char total_string[STRING_LEN];
	struct in_addr addr;
	struct hostent *host;
	struct sockaddr_in echoServAddr;
	struct sockaddr_in echoClntAddr;

	ip = "127.0.0.1";
	port = PORT;

	if (argc > 1) {

		ip = " ";
		port = 0;
		char *token = NULL;

		// token will point to the hostname.
		token = strtok(argv[1], ":");

		host = gethostbyname(token);
		if (host == NULL) {
			fprintf(stderr, "Gethostbyname() failed.\n");
			exit(EXIT_FAILURE);
		}
		ip = inet_ntoa(*(struct in_addr*) host->h_addr);

		// token will point to the port.
		token = strtok(NULL, ":");
		port = atoi(token);
	}

	if (port < 0) {
		printf("Bad port number %d\n", port);
		fflush(stdout);
		return 0;
	}

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		errorhandler("Socket() failed.\n");
		fflush(stdout);
	}

	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_port = htons(port);
	echoServAddr.sin_addr.s_addr = inet_addr(ip);

	if ((bind(sock, (struct sockaddr*) &echoServAddr, sizeof(echoServAddr)))
			< 0) {
		errorhandler("bind() failed.\n");
		fflush(stdout);
	}
	while (1) {
		cliAddrLen = 0;
		recvMsgSize = 0;

		operation.op = ' ';
		memset(total_string, 0, sizeof(total_string));

		while (operation.op != '=') {

			cliAddrLen = sizeof(echoClntAddr);

			recvMsgSize = recvfrom(sock, (char*) &operation, sizeof(operation),
					0, (struct sockaddr*) &echoClntAddr, &cliAddrLen);

			host = gethostbyaddr((char*) &addr, 4, AF_INET);
			char *canonical_name = host->h_name;

			if (operation.op != '=') {

				printf("Request operation '%c %d %d' by client %s, ip: %s\n",
						operation.op, operation.a, operation.b, canonical_name,
						inet_ntoa(echoClntAddr.sin_addr));
				fflush(stdout);
				switch (operation.op) {
				case '+':
					tot = add(operation.a, operation.b);
					break;
				case '-':
					tot = sub(operation.a, operation.b);
					break;
				case 'x':
					tot = mult(operation.a, operation.b);
					break;
				case '/':
					tot = division(operation.a, operation.b);
					if (operation.b == 0) {
						b = true;
					}
					break;
				}
				if (operation.op == '/' && b == false) {
					sprintf(total_string, "%.2f", tot);
				} else {
					//b is true if there is a division by a number and 0, false otherwise
					if (b == true) {
						strcpy(total_string, "error");
						b = false;
					} else {
						sprintf(total_string, "%d", (int) tot);
					}

				}

				if (sendto(sock, total_string, sizeof(total_string), 0,
						(struct sockaddr*) &echoClntAddr, sizeof(echoClntAddr))
						!= sizeof(total_string)) {
					errorhandler(
							"sendto() sent different number of bytes than expected.\n");
					fflush(stdout);
				}

			}
			operation.op = ' ';
			memset(total_string, 0, sizeof(total_string));

		}
	}
}
