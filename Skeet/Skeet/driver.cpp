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
#include "common_net.cpp"
#include "common.cpp"
#include "client_net.cpp"

#pragma comment(lib, "WS2_32")
#pragma warning(disable:4996)

struct Input
{
	bool32 space, left, right;
};

static bool32 g_is_running;
static Input g_input;

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
int main(int argc, char ** argv)
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
	// init winsock
	if (!Net::init())
	{
		log_warning("Net::init failed\n", 1);
		return 0;
	}
	Net::Socket sock;
	if (!Net::socket_create(&sock))
	{
		log_warning("create_socket failed\n", 1);
		return 0;
	}

	uint8 buffer[c_socket_buffer_size];
	Net::IP_Endpoint server_endpoint = Net::ip_endpoint_create(127, 0, 0, 1, c_port);

	buffer[0] = (uint8)Client_Message::Join;
	if (!Net::socket_send(&sock, buffer, 1, &server_endpoint))
	{
		log_warning("join message failed to send\n", 1);
		return 0;
	}


	struct Player_State
	{
		float32 x, y, facing;
	};
	Player_State objects[c_max_clients];
	uint32 num_objects = 0;
	uint16 slot = 0xFFFF;

	Timing_Info timing_info = timing_info_create();


	// main loop
	g_is_running = 1;
	while (g_is_running)
	{

		// sample the clock at the beginning of each tick
		LARGE_INTEGER tick_start_time;
		QueryPerformanceCounter(&tick_start_time);

		// Windows messages from win32 programming resources
		MSG message;
		UINT filter_min = 0;
		UINT filter_max = 0;
		UINT remove_message = PM_REMOVE;
		// Grabs packets from the socket for as long as there are any available
		while (PeekMessage(&message, window_handle, filter_min, filter_max, remove_message))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}


		// Process Packets
		Net::IP_Endpoint from;
		uint32 bytes_received;
		// Packet Layout:
		// first byte is a 0 or 1, indicating if we were allowed to join
		// If that byte is 1, then the next 2 bytes are a uint16 for our slot
		while (Net::socket_receive(&sock, buffer, c_socket_buffer_size, &from, &bytes_received))
		{
			switch (buffer[0])
			{
			case Server_Message::Join_Result:
			{
				if (buffer[1])
				{
					memcpy(&slot, &buffer[2], sizeof(slot));
				}
				else
				{
					log_warning("server didn't let us in\n");
				}
			}
			break;

			// State packet contains all the player objects
			// read until we run out of bytes
			// Server sends the slot (ID) which isn't used yet
			// this field is to figure out which object the player actually owns
			case Server_Message::State:
			{
				num_objects = 0;
				uint32 bytes_read = 1;
				while (bytes_read < bytes_received)
				{
					uint16 id; // unused
					memcpy(&id, &buffer[bytes_read], sizeof(id));
					bytes_read += sizeof(id);

					memcpy(&objects[num_objects].x, &buffer[bytes_read], sizeof(objects[num_objects].x));
					bytes_read += sizeof(objects[num_objects].x);

					memcpy(&objects[num_objects].y, &buffer[bytes_read], sizeof(objects[num_objects].y));
					bytes_read += sizeof(objects[num_objects].y);

					memcpy(&objects[num_objects].facing, &buffer[bytes_read], sizeof(objects[num_objects].facing));
					bytes_read += sizeof(objects[num_objects].facing);

					++num_objects;
				}
			}
			break;
			}
		}

		// Send input
		// input is sent every tick
		// key events sent to the window, information is stored in global g_input struct
		// pakcet has four buttons encoded in bitfield
		if (slot != 0xFFFF)
		{
			buffer[0] = (uint8)Client_Message::Input;
			int bytes_written = 1;

			memcpy(&buffer[bytes_written], &slot, sizeof(slot));
			bytes_written += sizeof(slot);

			uint8 input = (uint8)g_input.up |
				((uint8)g_input.down << 1) |
				((uint8)g_input.left << 2) |
				((uint8)g_input.right << 3);
			buffer[bytes_written] = input;
			++bytes_written;

			if (!Net::socket_send(&sock, buffer, bytes_written, &server_endpoint))
			{
				log_warning("socket_send failed\n");
			}
		}


		// Draw
		// run through each object and read from the most recent state packet
		// compute the 4 vertex positions to represent it
		for (uint32 i = 0; i < num_objects; ++i)
		{
			constexpr float32 size = 0.05f;
			float32 x = objects[i].x * 0.01f;
			float32 y = objects[i].y * -0.01f;

			uint32 verts_start = i * 4;
			vertices[verts_start].pos_x = x - size; // TL (hdc y is +ve down screen)
			vertices[verts_start].pos_y = y - size;
			vertices[verts_start + 1].pos_x = x - size; // BL
			vertices[verts_start + 1].pos_y = y + size;
			vertices[verts_start + 2].pos_x = x + size; // BR
			vertices[verts_start + 2].pos_y = y + size;
			vertices[verts_start + 3].pos_x = x + size; // TR
			vertices[verts_start + 3].pos_y = y - size;
		}
		// zero the x and y positions of all vertices for player objects
		// which have no actual player to show
		for (uint32 i = num_objects; i < c_max_clients; ++i)
		{
			uint32 verts_start = i * 4;
			for (uint32 j = 0; j < 4; ++j)
			{
				vertices[verts_start + j].pos_x = vertices[verts_start + j].pos_y = 0.0f;
			}
		}
		// pass the updated vertices to the graphics system to update and draw
		// finally wait for the tick to end and start over
		Graphics::update_and_draw(vertices, c_num_vertices, &graphics_state);


		wait_for_tick_end(tick_start_time, &timing_info);
	}

	// send message to the server that we're leaving
	// if this isn't sent then the server times out
	buffer[0] = (uint8)Client_Message::Leave;
	int bytes_written = 1;
	memcpy(&buffer[bytes_written], &slot, sizeof(slot));
	Net::socket_send(&sock, buffer, bytes_written, &server_endpoint);

	// todo( jbr ) return wParam of WM_QUIT
	return 0;
}

	// WSACleanup();
	return;

	//Game game(topLeft, bottomRight);
	//ui.run(callBack, &game);



	return ;
}
