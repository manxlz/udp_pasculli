/*
 ============================================================================
 Name        : client_udp_pasculli.c
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
#include <stdlib.h>
#include "protocol.h"

void errorhandler(char *errorMessage) {
	printf(errorMessage);
}

void clearwinSock() {
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
	int sock = 0;
	int port = 0;
	int fromSize = 0;
	int respStringLen = 0;
	char string_input;
	char total_string[STRING_LEN];
	char string1_input[STRING_LEN];
	char string2_input[STRING_LEN];
	struct in_addr addr;
	struct hostent *host;
	struct sockaddr_in fromAddr;
	struct sockaddr_in echoServAddr;
	const char *ip;

	ip = "127.0.0.1";
	port = PORT;

	if (argc > 1) {
		ip = " ";
		port = 0;
		char *pointer = NULL;

		// pointer will point to the hostname.
		pointer = strtok(argv[1], ":");

		host = gethostbyname(pointer);
		if(host == NULL){
			fprintf(stderr,"Gethostbyname() failed.\n");
			exit(EXIT_FAILURE);
		}
		ip = inet_ntoa(*(struct in_addr*) host->h_addr);

		// pointer will point to the port.
		pointer = strtok(NULL, ":");
		port = atoi(pointer);
	}

	if (port < 0) {
		printf("Bad port number %d\n", port);
		fflush(stdout);
		return 0;
	}

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		errorhandler("socket() failed.\n");
		fflush(stdout);
	}

	memset(&echoServAddr, 0, sizeof(echoServAddr));

	echoServAddr.sin_family = PF_INET;
	echoServAddr.sin_port = htons(port);
	echoServAddr.sin_addr.s_addr = inet_addr(ip);

	while (operation.op != '=') {

		//repeat itself if one or both of the two numbers is longer than STRING_LEN (8)
		do {
			printf(
					"Please insert the operation and two integer in this way: + 23 45\n");
			fflush(stdout);
			printf(
					"The operations are + for add, - for subtraction, x for multiply and / for divide\n");
			fflush(stdout);

			printf("To finish the program enter:= 0 0\n");
			fflush(stdout);

			scanf("%c %s %s", &string_input, string1_input, string2_input);
			fflush(stdin);

			if (strlen(string1_input) > STRING_LEN
					|| strlen(string2_input) > STRING_LEN) {
				printf("Input is too long. Please try again.\n");
				fflush(stdout);
				fflush(stdout);
				printf("\n");
			}
		} while (strlen(string1_input) > STRING_LEN
				|| strlen(string2_input) > STRING_LEN);

		if (operation.op != '=') {

			if (string_input == '+' || string_input == '-'
					|| string_input == 'x' || string_input == '/'
					|| string_input == '=') {
				operation.op = string_input;

				operation.a = atoi((char*) &string1_input);
				operation.b = atoi((char*) &string2_input);

				if (operation.op != '=') {
					if (sendto(sock, (char*) &operation, sizeof(operation), 0,
							(struct sockaddr*) &echoServAddr,
							sizeof(echoServAddr)) != sizeof(operation)) {
						errorhandler(
								"sendto() sent different number of bytes than expected.\n");
						fflush(stdout);

					}
					fromSize = sizeof(fromAddr);
					respStringLen = recvfrom(sock, total_string,
							sizeof(total_string), 0,
							(struct sockaddr*) &fromAddr, &fromSize);

					addr.s_addr = inet_addr(ip);
					host = gethostbyaddr((char*) &addr, 4, AF_INET);
					char *canonical_name = host->h_name;

					//total_string contains error beacuse there is a division by zero
					if (strcmp(total_string, "error") == 0) {
						printf(
								"The server %s, ip: %s says that the operation %d %c %d is impossible because you can't divide a number by zero\n",
								canonical_name, ip, operation.a, operation.op,
								operation.b);
						fflush(stdout);
						printf("\n");
						fflush(stdout);
					} else {

						printf(
								"Received result by server %s, ip %s: %d %c %d = %s\n",
								canonical_name, ip, operation.a, operation.op,
								operation.b, total_string);
						fflush(stdout);
						printf("\n");
						fflush(stdout);
					}
					if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
						fprintf(stderr,
								"Error: received a packet from unknown source.\n");
						fflush(stdout);
						exit(EXIT_FAILURE);
					}
				}

			} else {
				printf("Please insert a correct operation and try again.\n");
				fflush(stdout);
				printf("\n");
				fflush(stdout);

			}

		}

	}
	fflush(stdout);
	closesocket(sock);
	clearwinSock();
	system("pause");
	return 0;

}
