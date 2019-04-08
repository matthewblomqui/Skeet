#include <math.h>
#include <WinSock2.h>
#include <Windows.h>
#include "variables.h"
#include <stdio.h> // for printf

#pragma comment(lib, "WS2_32")
#pragma comment(lib, "winmm")

// we will put a byte at the front of each client packet to tell the server what
// kind of message was received
enum class Client_Message : uint8
{
	Join,    // tell the server we are here
	Leave,   // tell the server we are leaving
	Input    // tell the server our user input
};

// tells what kind of response the server is giving
enum class Server_Message : uint8
{
	Join_Result, // tell the client they're able/unable to join
	State        // tell the client the game state
};

// used to create a list of IP addresses and ports of the clients
struct IP_Endpoint
{
	uint32 address;
	uint16 port;
};

bool operator==(const IP_Endpoint& a, const IP_Endpoint& b)
{
	return a.address == b.address && a.port == b.port;
}

// store the variables of each player
struct Player_State
{
	float32 x, y, facing, speed;
};

// get the player input
struct Player_Input
{
	bool32 up, down, left, right;
};

// allows us to timeout if the client exits without telling the server
static float32 time_since(LARGE_INTEGER t, LARGE_INTEGER frequency)
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	return float32(now.QuadPart - t.QuadPart) / float32(frequency.QuadPart);
}


void main()
{
	WORD winsock_version = 0x202;
	WSADATA winsock_data;

	printf("Hello Server!\n");

	// make sure winsock started correctly
	if (WSAStartup(winsock_version, &winsock_data))
	{
		printf("WSAStartup failed: %d", WSAGetLastError());
		return;
	}


	// now create a socket
	int address_family = AF_INET;
	int type = SOCK_DGRAM;

	// we will be using UDP
	int protocol = IPPROTO_UDP;

	SOCKET sock = socket(address_family, type, protocol);

	// check for errors
	if (sock == INVALID_SOCKET)
	{
		printf("socket failed: %d", WSAGetLastError());
		return;
	}

	// choose a specific port to bind our soccet to. It has to be above 1024.
	// for now, we will use 9999
	SOCKADDR_IN local_address;
	local_address.sin_family = AF_INET;

	// htons converts the port number to big-endian or network byte order
	local_address.sin_port = htons(PORT);

	// INADDR_ANY allows us to bind to any address
	local_address.sin_addr.s_addr = INADDR_ANY;

	// if we were unable to bind
	if (bind(sock, (SOCKADDR*)&local_address, sizeof(local_address)) == SOCKET_ERROR)
	{
		printf("bind failed: %d", WSAGetLastError());
		return;
	}

	// put the socket in non-blocking mode
	// this makes it so the server doesn't wait on incoming packets
	u_long enabled = 1;
	ioctlsocket(sock, FIONBIO, &enabled);

	// make sure the game runs at the same speed 60hz on different devices
	UINT sleep_granularity_ms = 1;
	bool32 sleep_granularity_was_set = timeBeginPeriod(sleep_granularity_ms) == TIMERR_NOERROR;

	LARGE_INTEGER clock_frequency;
	QueryPerformanceFrequency(&clock_frequency);

	// This is our coude to create buffers for two players so we can start receiving
	// packets
	int8 buffer[SOCKET_BUFFER_SIZE];
	IP_Endpoint client_endpoints[MAX_CLIENTS];
	float32 time_since_heard_from_clients[MAX_CLIENTS];
	Player_State client_objects[MAX_CLIENTS];
	Player_Input client_inputs[MAX_CLIENTS];

	for (uint16 i = 0; i < MAX_CLIENTS; ++i)
	{
		client_endpoints[i] = {};
	}

	bool32 is_running = 1;

	// now we start our loop to receive packets from both clients
	while (is_running)
	{
		LARGE_INTEGER tick_start_time;
		QueryPerformanceCounter(&tick_start_time);

		while (true)
		{
			// get input packet for clients position
			int flags = 0;
			SOCKADDR_IN from;
			int from_size = sizeof(from);
			int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from,
				&from_size);

			// if there is a problem while receiving
			if (bytes_received == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					printf("recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", error);
				}

				break;
			}

			IP_Endpoint from_endpoint;
			from_endpoint.address = from.sin_addr.S_un.S_addr;
			from_endpoint.port = from.sin_port;

			switch (buffer[0])
			{
			case (int)Client_Message::Join:
			{
				printf("Client_Message::Join from %u:%hu\n", from_endpoint.address, from_endpoint.port);

				// look for an empty slot by finding an IP_Endpoint with an address of 0
				uint16 slot = uint16(-1);
				for (uint16 i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_endpoints[i].address == 0)
					{
						// if we found a slot, set the client ID to the available slot
						slot = i;
						break;
					}
				}

				buffer[0] = (int8)Server_Message::Join_Result;

				// send a join message to the client
				if (slot != uint16(-1))
				{
					printf("client will be assigned to slot %hu\n", slot);
					buffer[1] = 1;
					memcpy(&buffer[2], &slot, 2);

					flags = 0;
					if (sendto(sock, buffer, 4, flags, (SOCKADDR*)&from, from_size) != SOCKET_ERROR)
					{
						client_endpoints[slot] = from_endpoint;
						time_since_heard_from_clients[slot] = 0.0f;
						client_objects[slot] = {};
						client_inputs[slot] = {};
					}
					else
					{
						printf("sendto failed: %d\n", WSAGetLastError());
					}
				}
				// we have already hit our max number of players
				else
				{
					printf("could not find a slot for player\n");
					buffer[1] = 0;

					flags = 0;
					// we were unable to send a message to that client
					if (sendto(sock, buffer, 2, flags, (SOCKADDR*)&from, from_size) == SOCKET_ERROR)
					{
						printf("sendto failed: %d\n", WSAGetLastError());
					}
				}

			}
			break;

			// a client is trying to leave
			case (int)Client_Message::Leave:
			{
				uint16 slot;
				memcpy(&slot, &buffer[1], 2);

				if (client_endpoints[slot] == from_endpoint)
				{
					// zero the IP endpoint for the client
					client_endpoints[slot] = {};
					printf("Client_Message::Leave from %hu(%u:%hu)\n", slot, from_endpoint.address, from_endpoint.port);
				}
			}
			break;

			// the client is trying to send us input
			case (int)Client_Message::Input:
			{
				uint16 slot;
				memcpy(&slot, &buffer[1], 2);

				printf("%d %hu\n", bytes_received, slot);

				if (client_endpoints[slot] == from_endpoint)
				{
					// grab the input...
					uint8 input = buffer[3];

					// ...and convert it back to normal key presses
					client_inputs[slot].up = input & 0x1;
					client_inputs[slot].down = input & 0x2;
					client_inputs[slot].left = input & 0x4;
					client_inputs[slot].right = input & 0x8;

					time_since_heard_from_clients[slot] = 0.0f;

					printf("Client_Message::Input from %hu:%d\n", slot, int32(input));
				}
				// we discard the input because it was unexpected
				else
				{
					printf("Client_Message::Input discarded, was from %u:%hu but expected %u:%hu\n",
						from_endpoint.address, from_endpoint.port, client_endpoints[slot].address, client_endpoints[slot].port);
				}
			}
			break;
			}
		}

		// process input and update state
		// iterate through all IP_Endpoints in use
		for (uint16 i = 0; i < MAX_CLIENTS; ++i)
		{
			if (client_endpoints[i].address)
			{
				if (client_inputs[i].up)
				{
					client_objects[i].speed += ACCELERATION * SECONDS_PER_TICK;
					if (client_objects[i].speed > MAX_SPEED)
					{
						client_objects[i].speed = MAX_SPEED;
					}
				}
				if (client_inputs[i].down)
				{
					client_objects[i].speed -= ACCELERATION * SECONDS_PER_TICK;
					if (client_objects[i].speed < 0.0f)
					{
						client_objects[i].speed = 0.0f;
					}
				}
				if (client_inputs[i].left)
				{
					client_objects[i].facing -= TURN_SPEED * SECONDS_PER_TICK;
				}
				if (client_inputs[i].right)
				{
					client_objects[i].facing += TURN_SPEED * SECONDS_PER_TICK;
				}

				client_objects[i].x += client_objects[i].speed * SECONDS_PER_TICK * sinf(client_objects[i].facing);
				client_objects[i].y += client_objects[i].speed * SECONDS_PER_TICK * cosf(client_objects[i].facing);

				// check for timeouts
				time_since_heard_from_clients[i] += SECONDS_PER_TICK;
				if (time_since_heard_from_clients[i] > CLIENT_TIMEOUT)
				{
					printf("client %hu timed out\n", i);
					client_endpoints[i] = {};
				}
			}
		}

		// create a state packet
		// everyone gets the same one
		buffer[0] = (int8)Server_Message::State;
		int32 bytes_written = 1;

		for (uint16 i = 0; i < MAX_CLIENTS; ++i)
		{
			if (client_endpoints[i].address)
			{
				memcpy(&buffer[bytes_written], &i, sizeof(i));
				bytes_written += sizeof(i);

				memcpy(&buffer[bytes_written], &client_objects[i].x, sizeof(client_objects[i].x));
				bytes_written += sizeof(client_objects[i].x);

				memcpy(&buffer[bytes_written], &client_objects[i].y, sizeof(client_objects[i].y));
				bytes_written += sizeof(client_objects[i].y);

				memcpy(&buffer[bytes_written], &client_objects[i].facing, sizeof(client_objects[i].facing));
				bytes_written += sizeof(client_objects[i].facing);
			}
		}

		// send back to all clients
		int flags = 0;
		SOCKADDR_IN to;
		to.sin_family = AF_INET;
		to.sin_port = htons(PORT);
		int to_length = sizeof(to);

		for (uint16 i = 0; i < MAX_CLIENTS; ++i)
		{
			if (client_endpoints[i].address)
			{
				to.sin_addr.S_un.S_addr = client_endpoints[i].address;
				to.sin_port = client_endpoints[i].port;

				// we failed to send
				if (sendto(sock, buffer, bytes_written, flags, (SOCKADDR*)&to, to_length) == SOCKET_ERROR)
				{
					printf("sendto failed: %d\n", WSAGetLastError());
				}
			}
		}

		// wait until tick complete
		float32 time_taken_s = time_since(tick_start_time, clock_frequency);

		while (time_taken_s < SECONDS_PER_TICK)
		{
			if (sleep_granularity_was_set)
			{
				DWORD time_to_wait_ms = DWORD((SECONDS_PER_TICK - time_taken_s) * 1000);
				if (time_to_wait_ms > 0)
				{
					Sleep(time_to_wait_ms);
				}
			}

			time_taken_s = time_since(tick_start_time, clock_frequency);
		}
	}
}