#include <stdio.h>

#include "chatroom.h"
#include "resource.h"

// Global variable to share required data between functions
ChatRoom chatRoom;

// create a window with given dialog an procedure
static int dialog(int dlg, DLGPROC wndProc)
{
	// Create window
	chatRoom.hwnd = CreateDialog(
		chatRoom.hInstance,
		MAKEINTRESOURCE(dlg),
		NULL,
		wndProc);

	// error to create window
	if (!chatRoom.hwnd)
		return 1;

	// show window to user
	ShowWindow(chatRoom.hwnd, SW_NORMAL);
	UpdateWindow(chatRoom.hwnd);

	// wait and respond to user message's to window
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

// respond to messages to the connection dialog window
static LRESULT CALLBACK connection_dialog_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		// Connect button has been clicked
		case IDC_CONNECT_BT:
			if (connect_to_server() == 0) {
				// connection was succusfull
				// so close this window and continue in chatroom_start
				DestroyWindow(hwnd);
				PostQuitMessage(0);
			}
			// failed to connect
			SetDlgItemText(chatRoom.hwnd, IDC_STATUS, L"");
			break;
		}
        break;
	// user closed the window
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(1);
        break;
    }
    return 0;
}

// respond to messages to the create new account window
static LRESULT CALLBACK  signup_dialog_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		// Create button clicked
		case IDC_SU_CREATE:
			if (signUp() == 0) {
				DestroyWindow(hwnd);
				PostQuitMessage(ACCOUNT_OK);
			}
			break;
		// Login button clicked
		case IDC_SU_LOGIN:
			DestroyWindow(hwnd);
			PostQuitMessage(ACCOUNT_SIGNIN);
			break;
		}
		break;
	// user closed the window
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(ACCOUNT_QUIT);
		break;
	}
	return 0;
}

// respond to messages to the login window
static LRESULT CALLBACK  login_dialog_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		// Login button clicked
		case IDC_SI_LOGIN:
		{
			// try to log in
			int res = signIn();
			switch (res)
			{
			case 0:	// login was succussful. close this window and continue in chatroom_start function
				DestroyWindow(hwnd);
				PostQuitMessage(ACCOUNT_OK);
				break;
			case 1:	// failed to login
				MessageBox(NULL, L"Invalid username or password!", L"Login failed", MB_OK);
				break;
			default: // unexpected error.
				DestroyWindow(hwnd);
				break;
			}
		}
			break;
		case IDC_SI_CREATE:
		{
			// Create new account button clicked
			// hide this window
			ShowWindow(hwnd, SW_HIDE);
			// create new window for creating user
			int res = dialog(IDD_SIGNUP_DIALOG, (DLGPROC)signup_dialog_proc);
			switch (res) {
			// new account successfully created
			// close this window and continue in chatroom_start function
			case ACCOUNT_OK:
				DestroyWindow(hwnd);
				PostQuitMessage(ACCOUNT_OK);
				break;
			// back from create new user to login window
			case ACCOUNT_SIGNIN:
				ShowWindow(hwnd, SW_SHOW);
				break;
			// user closed create account window
			case ACCOUNT_QUIT:
				DestroyWindow(hwnd);
			}
		}
			break;
		}
		break;
	// user closed window
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(ACCOUNT_QUIT);
		break;
	}
	return 0;
}

// respond to messages to the chat window
static LRESULT CALLBACK chat_dialog_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	// window created
	case WM_INITDIALOG:
		// create a thread that receives messages from server
		chatRoom.rcvThreadHndle = CreateThread(0, 0, recv_message, NULL, 0, &chatRoom.rcvThreadID);
		if (chatRoom.rcvThreadHndle == NULL) {
			MessageBox(NULL, L"CreateThread Failed", L"ERROR", MB_ICONEXCLAMATION);
			DestroyWindow(hwnd);
			PostQuitMessage(1);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		// user clicked send button
		case IDC_SEND_BT:
		{
			// send message to server
			int res = send_message();
			// clear the message box
			SetDlgItemText(hwnd, IDC_MSG_BOX, L"");
			if (res) {
				// connection failed. close the program
				MessageBox(NULL, L"Connection failed!", L"Error", MB_ICONEXCLAMATION);
				DestroyWindow(hwnd);
				PostQuitMessage(res);
			}
		}
			break;
		}
		break;
	// user closed the program
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(1);
		break;
	}
	return 0;
}

// Start the chatroom
int chatroom_start(HINSTANCE hInstance)
{
	// Clear the chatRoom structure
    memset(&chatRoom, 0, sizeof(ChatRoom));

    chatRoom.hInstance = hInstance;

	// Initiate winsock
	WSADATA	d;
	if (WSAStartup(MAKEWORD(2, 2), &d)) {
		MessageBox(NULL, L"Failed to initialize winsock", L"Error", MB_ICONEXCLAMATION);
		return 1;
	}

	// create window that get's IP address and port number of server
	if (dialog(IDD_CONNECTION_DIALOG, (DLGPROC)connection_dialog_proc))
		return 1;		// uesr closed window

	// create login window
	int status = dialog(IDD_LOGIN_DIALOG, (DLGPROC)login_dialog_proc);
	if (status == ACCOUNT_QUIT)
		return 1;

	// create a semaphore to avoid writing in the chat box at the same time
	chatRoom.semaphore = CreateSemaphore(NULL, 1, 1, NULL);
	if (chatRoom.semaphore == NULL) {
		MessageBox(NULL, L"CreateSemaphore", L"Error", MB_ICONEXCLAMATION);
		return 1;
	}

	// create chat window
	if (dialog(IDD_CHAT_DIALOG, (DLGPROC)chat_dialog_proc))
		return 1;

	// terminate thread
	CloseHandle(chatRoom.rcvThreadHndle);
	// close connection
	closesocket(chatRoom.conn.sock_peer);
	// cleanup winsock
	WSACleanup();

	return 0;
}

int signUp()
{
	WCHAR	buff[USERNAME_LEN + 1];
	char	message[MESSAGE_LEN];
	char	username[USERNAME_LEN + 1];
	char	fpass[PASSWORD_LEN + 1];
	char	spass[PASSWORD_LEN + 1];
	int ulen, flen, slen;

	// extract username from window
	GetDlgItemText(chatRoom.hwnd, IDC_SU_USERNAME, buff, USERNAME_LEN);
	sprintf_s(username, "%ws", buff);
	ulen = strnlen(username, USERNAME_LEN);

	// extract first password from window
	GetDlgItemText(chatRoom.hwnd, IDC_SU_FPASSWORD, buff, PASSWORD_LEN);
	sprintf_s(fpass, "%ws", buff);
	flen = strnlen(fpass, PASSWORD_LEN);

	// extract second password from window
	GetDlgItemText(chatRoom.hwnd, IDC_SU_SPASSWORD, buff, PASSWORD_LEN);
	sprintf_s(spass, "%ws", buff);
	slen = strnlen(spass, PASSWORD_LEN);

	// fields should not be empty
	if (ulen == 0 || flen == 0 || slen == 0) {
		MessageBox(NULL, L"Username and password should not be empty", L"ERROR", MB_ICONEXCLAMATION);
		return 1;
	}

	// both passwords must be same
	if (strncmp(fpass, spass, PASSWORD_LEN) != 0) {
		MessageBox(NULL, L"Passwords does not match", L"ERROR", MB_ICONEXCLAMATION);
		return 1;
	}

	// create message to send to server
	sprintf_s(message, HDR_CREAT_STR"\r%s\r%s", username, fpass);

	// send message to server
	int numsent = send(chatRoom.conn.sock_peer, message, MESSAGE_LEN, 0);
	if (numsent < 1) {
		// server don't respond
		MessageBox(NULL, L"Server is unreachable!", L"ERROR", MB_ICONEXCLAMATION);
		return 1;
	}

	// receive server's respond
	int numread = recv(chatRoom.conn.sock_peer, message, MESSAGE_LEN, 0);
	// can't get any respond
	if (numread < 1)
		return 1;

	// Unable to create new user
	if (strncmp(message, HDR_DSAGR_STR, HEADER_LEN) == 0) {
		MessageBox(NULL, L"Unable to create this user!", L"ERROR", MB_ICONEXCLAMATION);
		return 1;
	}
	// This username already exist
	else if (strncmp(message, HDR_EXIST_STR, HEADER_LEN) == 0) {
		MessageBox(NULL, L"This username already exist!", L"ERROR", MB_ICONEXCLAMATION);
		return 1;
	}
	// New user created successfuly
	else if (strncmp(message, HDR_ACCPT_STR, HEADER_LEN) == 0)
		return 0;

	return 1;
}

// extract IP and PORT of the server from window
void get_ip_port()
{
	// buffer to hold data
	WCHAR buff[IP_LEN + 1];

	// store ip in buffer from IP field
	GetDlgItemText(chatRoom.hwnd, IDC_IP, buff, IP_LEN);
	sprintf_s(chatRoom.conn.ip, "%ws", buff);

	// store port number in buffer from PORT field
	GetDlgItemText(chatRoom.hwnd, IDC_PORT, buff, PORT_LEN);
	sprintf_s(chatRoom.conn.port, "%ws", buff);

	// set message of the label to "Connect to server..."
	SetDlgItemText(chatRoom.hwnd, IDC_STATUS, L"Connect to server...");
}

// connect to the esrver
int connect_to_server()
{
	// extract IP and PORT of the server from window
	get_ip_port();

	struct addrinfo hints;
	// clear structure
	memset(&hints, 0, sizeof(hints));
	// fill structure with given IP and port
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(chatRoom.conn.ip, chatRoom.conn.port, &hints, &chatRoom.conn.peer_addr)){
		MessageBox(NULL, L"getaddrinfo() failed", L"Error", MB_ICONEXCLAMATION);
		return 1;
	}

	// Create socket
	chatRoom.conn.sock_peer = socket(chatRoom.conn.peer_addr->ai_family,
		chatRoom.conn.peer_addr->ai_socktype,
		chatRoom.conn.peer_addr->ai_protocol);
	if (chatRoom.conn.sock_peer == INVALID_SOCKET) {
		MessageBox(NULL, L"socket() failed", L"Error", MB_ICONEXCLAMATION);
		return 1;
	}

	// clear structure
	if (connect(chatRoom.conn.sock_peer, chatRoom.conn.peer_addr->ai_addr, chatRoom.conn.peer_addr->ai_addrlen)) {
		MessageBox(NULL, L"connect() failed", L"Error", MB_ICONEXCLAMATION);
		return 1;
	}

	// free allocated memory for struct addrinfo hints
	freeaddrinfo(chatRoom.conn.peer_addr);

	return 0;
}

// Send chat messages to the server
int send_message()
{
	// Buffer to hold chat message
	WCHAR buffer[BUFFER_LEN + 1] = { 0 };
	// extract chat message from chat window
	GetDlgItemText(chatRoom.hwnd, IDC_MSG_BOX, buffer, BUFFER_LEN);
	// chat message can't be empty
	if (wcslen(buffer) == 0)
		return 0;

	// Wait if receive message thread is writing new message to chat textBox
	DWORD dwWaitResult = WaitForSingleObject( chatRoom.semaphore, 0L);
	switch (dwWaitResult)
	{
	// we can send message and update the chat textBox
	case WAIT_OBJECT_0:
	{
		// cast WCHAT * to char *
		char message[MESSAGE_LEN];
		memset(message, 0, MESSAGE_LEN);
		sprintf_s(message, "%ws\n", buffer);

		// this lines append message to the chat text box
		HWND chatBox = GetDlgItem(chatRoom.hwnd, IDC_CHAT_BOX);
		int offset = GetWindowTextLength(chatBox);
		SetFocus(chatBox);
		SendMessageA(chatBox, EM_SETSEL, (WPARAM)offset, (LPARAM)offset);	 // set selection - end of text
		SendMessageA(chatBox, EM_REPLACESEL, 0, (LPARAM)message);			 // append text

		// create message to send to server
		sprintf_s(message, HDR_MCHAT_STR"\r%ws\n", buffer);
		int msg_len = strnlen_s(message, MESSAGE_LEN);
		// send message to server
		int numsent = send(chatRoom.conn.sock_peer, message, msg_len, 0);
		if (numsent < 1) {
			MessageBox(NULL, L"Server does not respond!", L"ERROR", MB_ICONEXCLAMATION);
			return 1;
		}

		// release semaphore
		if (!ReleaseSemaphore(chatRoom.semaphore, 1, NULL)) {
			MessageBox(NULL, L"ReleaseSemaphore error", L"ERROR", MB_ICONEXCLAMATION);
			return 1;
		}
	}
		break;
	case WAIT_TIMEOUT:
		// timeout in wait
		break;
	}

	return 0;
}

// Login proccess
int signIn()
{
	WCHAR username[USERNAME_LEN + 1];
	WCHAR password[PASSWORD_LEN + 1];
	char  message[MESSAGE_LEN];
	// clear buffer
	memset(message, 0, MESSAGE_LEN);

	// extract username and password from window
	GetDlgItemText(chatRoom.hwnd, IDC_SI_USERNAME, username, USERNAME_LEN);
	GetDlgItemText(chatRoom.hwnd, IDC_SI_PASSWORD, password, PASSWORD_LEN);

	// create message to send to server
	sprintf_s(message, HDR_LOGIN_STR"\r%ws\r%ws", username, password);
	// send message to server
	if (send(chatRoom.conn.sock_peer, message, MESSAGE_LEN, 0) < 1) {
		MessageBox(NULL, L"Server does not respond!", L"ERROR", MB_ICONEXCLAMATION);
		return 1;
	}
	if (recv(chatRoom.conn.sock_peer, message, MESSAGE_LEN, 0) < 1) {
		MessageBox(NULL, L"Server does not respond!", L"ERROR", MB_ICONEXCLAMATION);
		return 1;
	}

	// user logged in successfully
	if (strncmp(message, HDR_ACCPT_STR, HEADER_LEN) == 0)
		return 0;
	// login failed
	else if (strncmp(message, HDR_DSAGR_STR, HEADER_LEN) == 0)
		return 1;

	return 2;
}

// Thread that recives chat messages from server
DWORD WINAPI recv_message(LPVOID lpParameter)
{
	char message[MESSAGE_LEN];
	while(1) {
		// clear message buffer
		memset(message, 0, MESSAGE_LEN);
		// wait for new chat message
		int numread = recv(chatRoom.conn.sock_peer, message, MESSAGE_LEN, 0);
		if (numread < 1)
			return 1;
		// wait if user is sending chat message to server
		DWORD dwWaitResult = WaitForSingleObject(chatRoom.semaphore, 0L);
		switch (dwWaitResult)
		{
		// we can now update chat text box
		case WAIT_OBJECT_0:
		{
			// make sure that message is null terminated to avoid overflow
			message[strlen(message)+1] = '\0';

			// this lines append text to chat text box
			HWND chatBox = GetDlgItem(chatRoom.hwnd, IDC_CHAT_BOX);
			SetFocus(chatBox);
			int offset = GetWindowTextLength(chatBox);
			SendMessageA(chatBox, EM_SETSEL, (WPARAM)offset, (LPARAM)offset);	// set selection - end of text
			SendMessageA(chatBox, EM_REPLACESEL, 0, (LPARAM)message);			// append text

			// Realease semaphore
			if (!ReleaseSemaphore(chatRoom.semaphore, 1, NULL)) {
				MessageBox(NULL, L"ReleaseSemaphore error", L"ERROR", MB_ICONEXCLAMATION);
				return 1;
			}
		}
			break;
		case WAIT_TIMEOUT:
			// timeout in wait
			break;
		}
	}
	return 0;
}