// This file contains WIN32 API header files

#ifndef __HEADERS_H_
#define __HEADERS_H_

// for windows socket programming
#ifndef		_WIN32_WINNT
#define 	_WIN32_WINNT	0x0600
#endif

/*  HEADERS */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

/*  HEADERS */

// retuired library for socket programming
#pragma comment(lib, "ws2_32.lib")

#endif /* __HEADERS_H_ */
