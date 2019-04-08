/*****************************************************
 * File: Driver.cpp
 * Author: Br. Burton
 *
 * Description: This file contains the main function
 *  that starts the game and the callback function
 *  that specifies what methods of the game class are
 *  called each time through the game loop.
 ******************************************************/
#include "game.h"
#include "uiInteract.h"

// added for server stuff
#include <stdio.h>
#include <WinSock2.h>
#include <winsock.h>
#include <winsock2.h>
#include "variables.h"

#pragma comment(lib, "WS2_32")
#pragma warning(disable:4996)

 /*************************************
  * All the interesting work happens here, when
  * I get called back from OpenGL to draw a frame.
  * When I am finished drawing, then the graphics
  * engine will wait until the proper amount of
  * time has passed and put the drawing on the screen.
  **************************************/
void callBack(const Interface *pUI, void *p)
{
	Game *pGame = (Game *)p;

	pGame->advance();
	pGame->handleInput(*pUI);
	pGame->draw(*pUI);
}


/*********************************
 * Main is pretty sparse.  Just initialize
 * the game and call the display engine.
 * That is all!
 *********************************/
void main(int argc, char ** argv)
{
	Point topLeft(-200, 200);
	Point bottomRight(200, -200);

	Interface ui(argc, argv, "Skeet", topLeft, bottomRight);
	//printf("Hello Client\n");

	// put this down below to check if it works
	Game game(topLeft, bottomRight);
	ui.run(callBack, &game);


	// May have to use Select() to be able to run game and use UDP
	/****************************************************************/

	printf("Hello Client!\n");
	// setting up winsocket will be the same as on the server
	WORD winsock_version = 0x202;
	WSADATA winsock_data;
	if (WSAStartup(winsock_version, &winsock_data))
	{
		printf("WSAStartup failed: %d", WSAGetLastError());
		return;
	}

	int address_family = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
	SOCKET sock = socket(address_family, type, protocol);

	if (sock == INVALID_SOCKET)
	{
		printf("socket failed: %d", WSAGetLastError());
		return;
	}

	// send the data to the server
	SOCKADDR_IN server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	// inet_addr is depricated, so I should change this latter. 
	server_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	int8 buffer[SOCKET_BUFFER_SIZE];
	int32 player_1;
	int32 player_2;

	printf("type w, a, s, or d to move, q to quit\n");
	bool32 is_running = 1;
	while (is_running)
	{
		// get input
		scanf_s("\n%c", &buffer[0], 1);

		// send to server
		int buffer_length = 1;
		int flags = 0;
		SOCKADDR* to = (SOCKADDR*)&server_address;
		int to_length = sizeof(server_address);
		if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
		{
			printf("sendto failed: %d", WSAGetLastError());
			return;
		}

		// wait for reply
		flags = 0;
		SOCKADDR_IN from;
		int from_size = sizeof(from);
		int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);

		if (bytes_received == SOCKET_ERROR)
		{
			printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError());
			break;
		}

		// grab data from packet
		int32 read_index = 0;

		memcpy(&player_1, &buffer[read_index], sizeof(player_1));
		read_index += sizeof(player_1);

		memcpy(&player_2, &buffer[read_index], sizeof(player_2));
		read_index += sizeof(player_2);

		memcpy(&is_running, &buffer[read_index], sizeof(is_running));

		printf("x:%d, y:%d, is_running:%d\n", player_1, player_2, is_running);
	}

	printf("done");

	// WSACleanup();
	return;

	//Game game(topLeft, bottomRight);
	//ui.run(callBack, &game);



	return ;
}
