#include <stdio.h>
#include <string.h>

#include "server.h"
#include "user.h"

#define FILENAME	"Users.db"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description: Start entry of server program 
 * =====================================================================================
 */
int main(int argc, char *argv[])
{
	// store port number of server
	char port[6];

	// default port is 8080
	// but if argv[1] can be used to set a desired port number
	if (argc > 1)
		strncpy(port, argv[1], 5);
	else
		strncpy(port, "8080", 4);

	printf("Selected PORT: %s\n", port);

	// allocate memory for list of users
	List *users = (List *)malloc(sizeof(List));
	if (!users){
		perror("can't read users");
		return 1;
	}

	// initiate list of users
	list_init(users, user_destroy);
	if (!users)
		return 1;

	// read users from file and store them in users list
	if (load_users(FILENAME, users))
		return 1;

	// structure to hold IP and port to start the server
	struct addrinfo *bind_addr;

	// initialize winsock and configure IP address and port
	if (server_config(port, &bind_addr))
		return 1;

	// create and bind socket and listen for new connections
	server_start(bind_addr, users);

	// free allocated memory for users
	list_destroy(users);

	// cleanup winsock
	WSACleanup();
	
	return 0;
}
