#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <map>

#include "Packet.h"

#pragma comment(lib, "ws2_32")

using namespace std;

SOCKET ServerSocket;

map<SOCKET, Player> SessionList;

unsigned WINAPI SendThread(void* Arg)
{
	srand((unsigned int)time(nullptr));

	char Message[1024] = { 0, };
	// 나 로그인 한다. -> 즉, 언리얼에서 했던 작업인 Server, Req 관련한 역할입니다.
	PacketManager::PlayerData.ID = 0;
	PacketManager::PlayerData.X = rand() % 80;
	PacketManager::PlayerData.Y = rand() % 15;
	PacketManager::Type = EPacketType::C2S_Login;	// 서버로 로그인
	PacketManager::Size = 12;

	// Data + Header(4바이트) 원래는 이 아래의 코드처럼 Header 사이즈 까지 포함해서 주는 것이 좋습니다.
	// PacketManager::Size = 12 + 4;

	PacketManager::MakePacket(Message);

	// 다만, 우리는 연습이 목적이기에 Send 할 때 이런식으로 하는 방식도 사용 해보도록 합니다.
	int SendLength = send(ServerSocket, Message, PacketManager::Size + 4, 0);

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
	while (true)
	{
		char Header[4] = { 0, };
		int RecvLength = recv(ServerSocket, Header, 4, 0);

		unsigned short DataSize = 0;
		EPacketType PacketType = EPacketType::Max;

		// Disassemble Header(헤더 분해)
		memcpy(&DataSize, &Header[0], 2);
		memcpy(&PacketType, &Header[2], 2);

		// 자료가 뒤집혀서 올 것이기 때문에 이를 원래대로 되돌리는 작업
		DataSize = ntohs(DataSize);
		PacketType = (EPacketType)(ntohs((u_short)PacketType));

		char Data[1024] = { 0, };
		RecvLength = recv(ServerSocket, Data, DataSize, MSG_WAITALL);

		if (RecvLength > 0)
		{
			// Packet Type
			switch (PacketType)
			{
			case EPacketType::S2C_Login:
			{
				// 원래는 고유 ID 를 만들어야 하는데 여기서는 생략
				// 모바일에서는 wifi 가 바뀌면 IP 가 바뀌기 때문에 이 코드로는 모바일에서 사용하면 안됩니다.
				// 그래서 이를 방지하기 위해서는 실제로는 UUID 를 사용하는 편입니다.
				Player NewPlayer;
				memcpy(&NewPlayer.ID, &Data[0], 4);
				memcpy(&NewPlayer.X, &Data[4], 4);
				memcpy(&NewPlayer.Y, &Data[8], 4);

				//NewPlayer.ID = ntohl(NewPlayer.ID);
				NewPlayer.ID = ntohl(NewPlayer.ID);		// PlayerID 는 소켓의 넘버
				NewPlayer.X = ntohl(NewPlayer.X);
				NewPlayer.Y = ntohl(NewPlayer.Y);

				SessionList[NewPlayer.ID] = NewPlayer;

				cout << "Connect Client : " << SessionList.size() << endl;
			}
			break;

			case EPacketType::S2C_Spawn:
			{
				Player NewPlayer;
				memcpy(&NewPlayer.ID, &Data[0], 4);
				memcpy(&NewPlayer.X, &Data[4], 4);
				memcpy(&NewPlayer.Y, &Data[8], 4);

				//NewPlayer.ID = ntohl(NewPlayer.ID);
				NewPlayer.ID = ntohl(NewPlayer.ID);		// PlayerID 는 소켓의 넘버
				NewPlayer.X = ntohl(NewPlayer.X);
				NewPlayer.Y = ntohl(NewPlayer.Y);

				// 플레이어 정보의 끝 까지 갔으면 못 찾았다는 의미이니까 그 때에 대한 예외처리입니다.
				// 이를 하는 이유는 PlayerID 의 중복을 막기 위한 처리입니다.
				if (SessionList.find(NewPlayer.ID) != SessionList.end())
				{
					SessionList[NewPlayer.ID] = NewPlayer;
				}

				cout << "Spawn Client" << endl;
			}
			break;
			}
		}
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