#ifndef __CHATROOM_H_
#define __CHATROOM_H_

#include "headers.h"
#include "protocol.h"

// some inner messages int the program
#define ACCOUNT_OK			0
#define ACCOUNT_SIGNIN		1
#define ACCOUNT_QUIT		2

// structure that holds required data for connection to the server
typedef struct _connection {
	SOCKET				sock_peer;			// socket for send and resv messages
	struct addrinfo*	peer_addr;			// holds information of server to connect to
	char				ip[IP_LEN];			// IP address of sesrver
	char				port[PORT_LEN];		// Port number of server
} Connection;

// structure that holds requires data for using between functions in program
typedef struct _chatroom {
	HINSTANCE	hInstance;			// Instance of current exe
	HWND		hwnd;				// points to current woring window
	Connection	conn;				// required data for connections
	HANDLE		semaphore;			// a semaphore to avoid race conditions
	DWORD		rcvThreadID;		// Thread ID of the receiver thread function
	HANDLE		rcvThreadHndle;		// a handler to the thread that receives data from server
} ChatRoom;

// A global instance of ChatRoom structure. to share data between functions
extern ChatRoom chatRoom;

// Starts the program rutine
extern int chatroom_start(HINSTANCE hInstance);

// get IP and Port of the server from window and store them in chatRoom structure
extern void get_ip_port();

// connect to the server
extern int connect_to_server();

// send chat message to server
extern int send_message();

// login to the server
extern int signIn();

// create a new account
extern int signUp();

// a thread that recevies chats from server
extern DWORD WINAPI recv_message(LPVOID lpParameter);

#endif /* __CHATROOM_H_ */
