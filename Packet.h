#pragma once
#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstring>
#include <WinSock2.h>

enum class EPacketType : unsigned short
{
	C2S_Spawn = 100,
	S2C_Spawn,
	C2S_Login,
	S2C_Login,
	C2S_Logout,
	S2C_Logout,
	C2S_Move,
	S2C_Move,

	Max
};


class Player
{
public:
	int X;
	int Y;
	int ID;
};


class PacketManager
{
public:
	unsigned short Size = 0;
	EPacketType Type;
	Player PlayerData;
	
	char Buffer[1024];

	void MakeData()
	{
		// 자료를 만드는 형태는 아래와 같습니다. [] -> 1바이트
		// Size Type ID       X        Y
		// [][] [][] [][][][] [][][][] [][][][]
		// htons 를 사용한 이유는 Size 의 순서가 뒤바뀌면 결과값이 바뀌기 때문
		unsigned short Data = htons(Size);		// Player 정보 Size
		memcpy(&Buffer[0], &Data, 2);			// 2바이트 짜리 만들어서 Buffer 에 넣기

		Data = htons((unsigned short)(Type));	// Player 종류 Type
		memcpy(&Buffer[2], &Data, 2);			// 2바이트 짜리 만들어서 Buffer 에 넣기

		int Data2 = htons(PlayerData.ID);		// Player ID
		memcpy(&Buffer[4], &Data, 4);			// 4바이트 짜리 만들어서 Buffer 에 넣기

		Data2 = htons(PlayerData.X);			// Player X 좌표
		memcpy(&Buffer[8], &Data, 4);			// 4바이트 짜리 만들어서 Buffer 에 넣기

		Data2 = htons(PlayerData.Y);			// Player Y 좌표
		memcpy(&Buffer[12], &Data, 4);			// 4바이트 짜리 만들어서 Buffer 에 넣기
	}
};


#endif // __PACKET_H__