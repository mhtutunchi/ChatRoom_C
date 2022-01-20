#ifndef __USER_H_
#define __USER_H_

#include "protocol.h"
#include "list.h"

// structure that hods information about a users
typedef struct _user {
	char 	username[USERNAME_LEN+1];
	char 	password[HASHPASS_LEN+1];
	int  	logged;
	SOCKET 	sockfd;		// use to send msg to all
} User;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  save_users
 *  Description:  Save user's informations in the file
 * =====================================================================================
 */
extern int save_users(const char *filename, List *users);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  load_users
 *  Description:  Read user's informations from file and store them in users list
 * =====================================================================================
 */
extern int load_users(const char *filename, List *users);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  user_search_by_username
 *  Description:  Search for a specefic username in list of users
 * =====================================================================================
 */
extern User *user_search_by_username(List *users, const char *username);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  user_print
 *  Description: Print out information of all users in stdout. (for test) 
 * =====================================================================================
 */
extern void user_print(List *users);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  user_destroy
 *  Description: free allocated memory for a user. 
 * =====================================================================================
 */
extern void user_destroy(void *data);

#endif /*  __USER_H_ */
