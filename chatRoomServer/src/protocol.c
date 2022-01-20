#include <stdio.h>
#include <string.h>
#include "headers.h"
#include "protocol.h"

// Macto to compare two header strings
#define HDRCMP(H1, H2)		(strncmp(H1, H2, HEADER_LEN) == 0)

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  send_msg
 *  Description: Send a Header message through sockfd 
 * =====================================================================================
 */
int send_msg(SOCKET sockfd, const char *msg)
{
	int numsent = send(sockfd, msg, HEADER_LEN, 0);
	if (numsent < 1)
		return 1;
	return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_msg_code
 *  Description: Extract integer code of a given header message 
 * =====================================================================================
 */
int get_msg_code(const char *message)
{
	int code;
	if (HDRCMP(message, HDR_LOGIN_STR))
		return HDR_LOGIN_INT;
	if (HDRCMP(message, HDR_LOGOT_STR))
		return HDR_LOGOT_INT;
	if (HDRCMP(message, HDR_CREAT_STR))
		return HDR_CREAT_INT;
	if (HDRCMP(message, HDR_MCHAT_STR))
		return HDR_MCHAT_INT;
	if (HDRCMP(message, HDR_ACCPT_STR))
		return HDR_ACCPT_INT;
	if (HDRCMP(message, HDR_DSAGR_STR))
		return HDR_DSAGR_INT;
	return HDR_ERROR_INT;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  rem_msg_header
 *  Description: Remove header string from message 
 * =====================================================================================
 */
int rem_msg_header(char *message)
{
	char tmp[MESSAGE_LEN];
	int i, j;
	for (i = HEADER_LEN+1, j = 0; i < strnlen(message, MESSAGE_LEN); ++i, j++)
		tmp[j] = message[i];
	tmp[j] = '\0';
	strncpy(message, tmp, MESSAGE_LEN);
	return 0;
}
