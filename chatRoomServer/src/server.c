#include "headers.h"
#include "server.h"
#include "user.h"
#include "md5.h"

// A linked list that contains all users
List *users;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_client_info
 *  Description:  extract IP address and port number of client.
 *  and store that in a two readable strings.
 * =====================================================================================
 */
static int get_client_info(ThreadData *data)
{
	if (!data)			return 1;
	socklen_t client_len = sizeof(data->client_addr);
	getnameinfo((struct sockaddr *)&data->client_addr,
			client_len, data->ip, sizeof(data->ip),
			data->port, sizeof(data->port),
			NI_NUMERICHOST);
	return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  login
 *  Description: This function responds to login request of a client 
 * =====================================================================================
 */
static int login(char *message, ThreadData *data)
{
	// buffer to hold username
	char username[USERNAME_LEN];
	// buffer to hold password
	char password[HASHPASS_LEN+1];

	// remove header message HDR_LOGIN
	rem_msg_header(message);

	char *token;
	char del[2] = "\r";

	// extract username
	token = strtok(message, del);
	if (!token)		return 1;
	strncpy(username, token, USERNAME_LEN);

	// extract password
	token = strtok(NULL, del);
	if (!token)		return 1;
	md5(token, password);

	// search if this user exists.
	User *user = user_search_by_username(users, username);
	if (!user)	return 1;

	// if users exists. validate his\her password
	if (strncmp(user->password, password, HASHPASS_LEN) == 0){
		// user logged in successfully
		data->user = user;
		user->logged = 1;
		user->sockfd = data->sockfd;
		return 0;
	}

	return 1;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  logout
 *  Description: This function change state of a logged user to ofline. and close his socket
 * =====================================================================================
 */
void logout(ThreadData *data)
{
	printf("Client ");
	if (get_client_info(data) == 0){
		printf("%s:%s ", data->ip, data->port);
		if (data->user){
			if (data->user->logged)
				printf("(%s) ", data->user->username);
			data->user->logged = 0;
		}
	}
	printf("Disconnected\n");
	closesocket(data->sockfd);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  client_thread
 *  Description: This function is created for each thread and will be responsible for the connections  
 * =====================================================================================
 */
static DWORD WINAPI client_thread(LPVOID lpParameter)
{
	// cast and store thread data
	ThreadData *data = (ThreadData *)lpParameter;
	// get client ip and port number
	get_client_info(data);
	// buffer to hold messages
	char message[MESSAGE_LEN];
	int numread;
	int msg_code;

	printf("Client %s:%s Connected\n", data->ip, data->port);
	while(1){
		// clear buffer
		memset(message, 0, MESSAGE_LEN);
		// wait for new message
		numread = recv(data->sockfd, message, MESSAGE_LEN, 0);
		if (numread < 1){
			// something went wrong. so logout the clinet
			logout(data);
			break;
		}
		// extract message code from client
		msg_code = get_msg_code(message);
		// check what client wants
		server_respond(message, msg_code, data);
	}
	return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  server_config
 *  Description:  Initialize winsock and configure IP address and port
 * =====================================================================================
 */
int server_config(const char *port, struct addrinfo **bind_addr)
{
	// required to startup winsock
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d)) {
		socket_strerror("WSAStartup() failed");
		return 1;
	}

	printf("Ready to use socket API\n");
	printf("Configuring local address...\n");

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));	// fill hints structure with 0
	hints.ai_family = AF_INET;			// IPv4
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_flags = AI_PASSIVE;		// to let getaddrinfo assign IP

	// fill address and port in bind_addr
	if (getaddrinfo(0, port, &hints, bind_addr)){
		socket_strerror("getaddrinfo() failed");
		return 1;
	}
	return 0;
}

// we need this function to free allocated memory in list of threads
static void thread_destroy(void *data)
{
	free(data);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  server_start
 *  Description:  create and bind socket and listen for new connections
 * =====================================================================================
 */
int server_start(struct addrinfo *bind_addr, List *_users)
{
	if (!bind_addr || !_users)
		return 1;

	// store list of users in a global variable
	users = _users;

	printf("Creating socket...\n");
	SOCKET sock_listen;
	sock_listen = socket(bind_addr->ai_family,
			bind_addr->ai_socktype, bind_addr->ai_protocol);
	if (!socket_is_valid(sock_listen)){
		socket_strerror("socket() failed");
		return 1;
	}

	printf("Binding socket to local address...\n");
	if (bind(sock_listen, bind_addr->ai_addr, bind_addr->ai_addrlen)){
		socket_strerror("bind() failed");
		return 1;
	}

	// free allocated memory for bin_addr. we are done with that structure
	freeaddrinfo(bind_addr);
	
	printf("Listening...\n");
	if (listen(sock_listen, LISTEN_QUEUE) < 0){
		socket_strerror("listen() failed\n");
		closesocket(sock_listen);
		return 1;
	}

	List *threads = (List *)malloc(sizeof(List));
	list_init(threads, thread_destroy);
	if (!threads){
		fprintf(stderr, "allocate memory for threads list\n");
		closesocket(sock_listen);
		return 1;
	}

	// structure to hold information about connected client
	struct sockaddr_storage client_addr;
	socklen_t client_len = sizeof(client_addr);

	// main loop
	while(1){
		// wait for connection
		SOCKET sock_client = accept(sock_listen,
				(struct sockaddr *)&client_addr, &client_len);
		if (!socket_is_valid(sock_client)){
			socket_strerror("accept() failed\n");
			return 1;
		}
		// allocate memory for new thread
		ThreadData *td = (ThreadData *)malloc(sizeof(ThreadData));
		if (!td){
			socket_strerror("malloc() failed\n");
			closesocket(sock_client);
			continue;
		}
		// initialize thread data
		memset(td, 0, sizeof(td));
		td->sockfd = sock_client;
		td->client_addr = client_addr;

		ThreadInfo *ti = (ThreadInfo *)malloc(sizeof(ThreadInfo));
		if (!ti){
			socket_strerror("malloc() failed\n");
			free(td);
			socket_close(sock_client);
			continue;
		}

		// create a therad to respond to client
		ti->threadHandle = CreateThread(0, 0,
				client_thread, td, 0, &ti->threadID);
		list_ins_next(threads, NULL, ti);
	}

	// Terminate all threads
	ListElmt *th;
	ThreadInfo *td;
	for(th = list_head(threads); th != NULL; th = list_next(th)){
		td = (ThreadInfo *)list_data(th);
		TerminateThread(td->threadHandle, 0);
	}
	// free all allocated threadInfo's
	list_destroy(threads);

	return 0;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  send_chat_to_all
 *  Description: Sends a message to all users 
 * =====================================================================================
 */
static int send_chat_to_all(char *message, ThreadData *data)
{
	if (!message || !data)		return 1;

	// user should be logged in to send chat
	if (data->user->logged == 0)
		return 1;

	// rempve header message from message
	rem_msg_header(message);
	
	// buffer to add username to message
	char msg[MESSAGE_LEN];
	strncpy(msg, data->user->username, USERNAME_LEN);
	strncat(msg, ": ", 3);
	strncat(msg, message, BUFFER_LEN);

	// iterate all users and send the message to all users. except to the sender
	ListElmt 	*elm;
	for (elm = list_head(users); elm != NULL; elm = list_next(elm)){
		User *user = (User *)list_data(elm);
		// we don't want to send message to sender
		if (user->logged && user->sockfd != data->sockfd)
			send(user->sockfd, msg, MESSAGE_LEN, 0) < strlen(msg);
	}
	return 0;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  create_new_account
 *  Description: create a new account.
 *  return 0 for succuss. 1 for error and 2 if username already exits.
 * =====================================================================================
 */
static int create_new_account(char *message, ThreadData *data)
{
	rem_msg_header(message);

	char *token;
	char del[2] = "\r";

	// extract username from message
	token = strtok(message, del);
	if (!token)		return 1;

	// allocate memory for new user
	User *new_user = (User *)malloc(sizeof(User));
	if (!new_user)
		return 1;
	strncpy(new_user->username, token, USERNAME_LEN);

	// extract password from message
	token = strtok(NULL, del);
	if (!token){
		free(new_user);
		return 1;
	}

	// hash password and store that in new_user->password
	md5(token, new_user->password);

	// check if this username already exist
	if (user_search_by_username(users, new_user->username) != NULL){
		free(new_user);
		return 2;		// return 2: this username already exists
	}

	// add this user to users list
	if (list_ins_next(users, NULL, new_user) != 0){
		free(new_user);
		return 1;
	}

	// save all users in the file
	save_users("Users.db", users);

	// change login state of this new user to online
	new_user->logged = 1;
	new_user->sockfd = data->sockfd;
	data->user = new_user;

	printf("New account created: %s\n", data->user->username);
	return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  server_respond
 *  Description:  Translate message and respond to that message.
 * =====================================================================================
 */
int server_respond(char *message,
		const int msg_code, ThreadData *data)
{
	int res;			/*  to send proper message to client */
	switch(msg_code)
	{
		// client wants to login
		case HDR_LOGIN_INT:
			if (login(message, data))
				return send_msg(data->sockfd, HDR_DSAGR_STR);
			return send_msg(data->sockfd, HDR_ACCPT_STR);
			break;
		// client wants to logout
		case HDR_LOGOT_INT:
			logout(data);
			break;
		// client wants to create account
		case HDR_CREAT_INT:
			res = create_new_account(message, data);
			// 1: error to create new account
			if (res == 1)
				return send_msg(data->sockfd, HDR_DSAGR_STR);
			// 2 this username already exist
			if (res == 2)
				return send_msg(data->sockfd, HDR_EXIST_STR);
			// new account created succussfuly. send HDT_ACCPT message to client
			return send_msg(data->sockfd, HDR_ACCPT_STR);
			break;
		// client wants to send a message to all users
		case HDR_MCHAT_INT:
			send_chat_to_all(message, data);
			break;
		default:
			break;
	}
	return 0;
}
