#ifndef __PEROTOCOL_H_
#define __PROTOCOL_H_

#include "headers.h"

// structure of a message
/*  0----------8-------------------308------310---311 *
 *  |  HEADER  |      CHATMSG       |  CRLN  | \0  | 
 *  ------------------------------------------------ */

// Constants
#define COLOMN			2		// ": "
#define CRLN			2
#define HEADER_LEN		9
#define CHATMSG_LEN		300
#define BUFFER_LEN		(CHATMSG_LEN + CRLN)
#define MESSAGE_LEN		(USERNAME_LEN + BUFFER_LEN + COLOMN) + 1

// Header Message codes
#define HDR_ERROR_INT		-1
#define HDR_LOGIN_INT		0
#define HDR_LOGOT_INT		1
#define HDR_CREAT_INT		2
#define HDR_EXIST_INT		3
#define HDR_MCHAT_INT		4
#define HDR_ACCPT_INT		5
#define HDR_DSAGR_INT		6

// Header Message strings
#define HDR_LOGIN_STR		"HDR_LOGIN"
#define HDR_LOGOT_STR		"HDR_LOGOT"
#define HDR_CREAT_STR		"HDR_CREAT"
#define HDR_EXIST_STR		"HDR_EXIST"
#define HDR_MCHAT_STR		"HDR_MCHAT"
#define HDR_ACCPT_STR		"HDR_ACCPT"
#define HDR_DSAGR_STR		"HDR_DSAGR"

// Length of buffers
#define USERNAME_LEN	50
#define PASSWORD_LEN	50
#define HASHPASS_LEN	32

#define IP_LEN			15
#define PORT_LEN		5

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  send_msg
 *  Description: send a given message through sockfd 
 * =====================================================================================
 */
extern int send_msg(SOCKET sockfd, const char *msg);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_msg_code
 *  Description: extract code of a header message. each header message have a string and integer code. this integer codes are easy to user. rather than strings. but we send strings through network 
 * =====================================================================================
 */
extern int get_msg_code(const char *message);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  rem_msg_header
 *  Description: Remove header from message 
 * =====================================================================================
 */
extern int rem_msg_header(char *message);

#endif /*  __PROTOCOL_H_  */
