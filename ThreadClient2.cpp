#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

#pragma comment(lib, "ws2_32")

SOCKET ServerSocket;

unsigned WINAPI SendThread(void* Arg)
{
	char Message[1024] = { 0, };

	while (true)
	{
		// getline 은 띄어쓰기 인식
		std::cin.getline(Message, sizeof(Message));

		int SendLength = send(ServerSocket, Message, (int)strlen(Message), 0);
	}

	return 0;
}

// Blocking 함수의 역할을 합니다.
// Recv 즉, 이미 사용중이라면 해당 스레드의 작업이 끝날때까지 기다려야 하기 때문입니다.
unsigned WINAPI RecvThread(void* Arg)
{
	char Buffer[1024] = { 0, };

	while (true)
	{
		int RecvLength = recv(ServerSocket, Buffer, sizeof(Buffer), 0);

		std::cout << Buffer << std::endl;
	}

	return 0;
}

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ServerSockAddr = { 0 , };
	ServerSockAddr.sin_family = AF_INET;
	ServerSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerSockAddr.sin_port = htons(22222);

	connect(ServerSocket, (SOCKADDR*)&ServerSockAddr, sizeof(ServerSockAddr));

	HANDLE ThreadHandles[2];

	ThreadHandles[0] = (HANDLE)_beginthreadex(0, 0, SendThread, 0, 0, 0);
	ThreadHandles[1] = (HANDLE)_beginthreadex(0, 0, RecvThread, 0, 0, 0);

	// 스레드가 계속 돌아가게 하기 위해서
	while (true)
	{
		Sleep(1);
	}

	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}