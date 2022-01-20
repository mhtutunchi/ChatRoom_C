 /*
 * Simple MD5 implementation
 *
 * Compile with: gcc -o md5 md5.c
 */

#ifndef __MD5_H_
#define __MD5_H_

#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

extern void md5(const char *msg, char *hashed);

#endif 	/*  __MD5_H_ */
