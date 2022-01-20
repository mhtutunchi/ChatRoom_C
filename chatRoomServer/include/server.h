#ifndef __SERVER_H_
#define __SERVER_H_

#include <stdio.h>

#include "headers.h"
#include "user.h"

#define 	LISTEN_QUEUE		50

#define		socket_is_valid(s)	((s) != INVALID_SOCKET)
#define 	socket_close(s)		closesocket(s)
#define 	socket_strerror(str)		\
		fprintf(stderr, str": %d\n", WSAGetLastError())

// structure that hold required data for a thread
typedef struct _threadData {
	SOCKET 		sockfd;					 // client socket
	User 		*user;					 // a pointer to user
	char  		ip[IP_LEN+1];			 // ip address of client
	char   		port[PORT_LEN+1];		 // ip port of the client
	struct sockaddr_storage client_addr; // struture that holds IP:PORT of client.
} ThreadData;

typedef struct _threadInfo {
	DWORD 		threadID;
	HANDLE		threadHandle;
} ThreadInfo;

/* 
 * ===  FUNCTION  =========================================================
 *         Name:  server_config
 *  Description:  Initialize winsock and configure IP address and port
 * ========================================================================
 */
extern int server_config(const char *port, struct addrinfo **bind_addr);

/* 
 * ===  FUNCTION  =========================================================
 *         Name:  server_start
 *  Description:  Create and bind socket and listen for new connections
 * ========================================================================
 */
extern int server_start(struct addrinfo *bind_addr, List *_users);

/* 
 * ===  FUNCTION  =========================================================
 *         Name:  server_respond
 *  Description:  Translate incomming message and respond to that message.
 * ========================================================================
 */
extern int server_respond(char *message,
		const int msg_code, ThreadData *data);

#endif /*  __SERVER_H_  */
