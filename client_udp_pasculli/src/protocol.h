/*
 * protocol.h
 *
 *  Created on: 14 dic 2021
 *      Author: Emanuele Pasculli
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define PORT 48000
#define STRING_LEN 8

typedef struct {
	char op;
	int a;
	int b;
} operationStruct;


#endif /* PROTOCOL_H_ */
