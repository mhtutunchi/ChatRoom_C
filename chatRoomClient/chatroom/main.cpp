#include "chatroom.h"

// Entry point of client program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// start chatroom
	return chatroom_start(hInstance);
}