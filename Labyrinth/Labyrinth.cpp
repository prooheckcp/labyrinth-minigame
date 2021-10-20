/*CLIENT-SERVER MESSAGE PROTOCOL

Server sends a userData for the client to update

Client sends movement data

*/

//Includes||
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#include <string>
//________||

//Defines||
#define MAXPENDINGCONNECTIONS 10
#define MAXRECVBUFFER 1024
//_______||

//Namespaces||
using namespace std;
//__________||

//Constants||

//datatypes
const char DATA_TYPE_UPDATE = '\15';

const char DATA_BREAKER = '\20';
const char DATA_END = '\18';

const char WALL_CHAR = (char)178;

const int WORLD_SIZE = 20;
const char WORLD_MAP[WORLD_SIZE][WORLD_SIZE] =
{ { 'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X' },
 { 'X',' ','X',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','X','X',' ',' ','X' },
 { 'X',' ','X',' ',' ',' ',' ','X',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','X' },
 { 'X',' ','X','X','X',' ',' ','X',' ',' ','X','X','X',' ',' ','X',' ','X',' ','X' },
 { 'X',' ',' ',' ','X',' ',' ','X','X',' ','X',' ',' ',' ',' ','X',' ','X',' ','X' },
 { 'X',' ',' ',' ','X',' ',' ','X',' ',' ',' ',' ','X','X','X','X',' ',' ',' ','X' },
 { 'X',' ','X',' ','X','X',' ','X',' ',' ',' ',' ','X',' ',' ','X',' ','X',' ','X' },
 { 'X',' ',' ',' ',' ',' ',' ',' ',' ','X','X','X','X',' ',' ','X',' ','X',' ','X' },
 { 'X',' ',' ',' ','X','X','X',' ',' ',' ',' ',' ',' ',' ',' ','X',' ','X',' ','X' },
 { 'X',' ',' ',' ','X',' ',' ',' ','X','X','X',' ','X','X',' ','X','X','X',' ','X' },
 { 'X',' ',' ',' ',' ',' ','X',' ',' ',' ',' ',' ',' ','X',' ',' ',' ',' ',' ','X' },
 { 'X',' ',' ',' ','X',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','X' },
 { 'X','X',' ','X','X',' ','X','X','X','X','X',' ',' ','X',' ','X','X','X','X','X' },
 { 'X',' ',' ',' ',' ',' ','X',' ',' ',' ','X',' ',' ','X',' ','X',' ',' ',' ','X' },
 { 'X',' ',' ',' ',' ',' ',' ',' ','X',' ','X',' ',' ','X',' ','X',' ',' ',' ','X' },
 { 'X','X',' ','X',' ','X','X',' ','X',' ','X',' ','X','X',' ','X',' ','X',' ','X' },
 { 'X',' ',' ',' ',' ',' ',' ',' ','X','X','X',' ','X',' ',' ','X',' ','X',' ','X' },
 { 'X',' ',' ',' ',' ','X','X',' ',' ',' ',' ',' ','X','X','X','X',' ','X',' ','X' },
 { 'X',' ',' ','X',' ',' ',' ',' ','X','X',' ',' ',' ',' ',' ',' ',' ',' ',' ','X' },
 { 'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X' } };
//_________||

//Structs||
struct PlayerInfo {
	SOCKET client;
	int id;
	char avatar;
	int positionx;
	int positiony;
};

struct Vector2 {
	int x;
	int y;
};
//_______||

//Variables||
vector<PlayerInfo> players;
int playerCount = 0;
char playerAvatars = 'A';
//_________||

void DrawWorld() {
	system("cls");
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {
			if (WORLD_MAP[x][y] == 'X')
				cout << WALL_CHAR;
			else {
				bool foundPlayer = false;
				for (int i = 0; i < players.size(); i++) {
					PlayerInfo player = players.at(i);
					if (player.positionx == x && player.positiony == y) {
						cout << player.avatar;
						foundPlayer = true;
						break;
					}
				}
				if (!foundPlayer)
					cout << ' ';
			}
		}
		cout << endl;
	}
}

Vector2 GetEmptySpot() {
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {
			char worldSpot = WORLD_MAP[x][y];
			if (worldSpot == ' ') {
				bool foundPlayer = false;
				for (int i = 0; i < players.size(); i++) {
					PlayerInfo player = players.at(i);
					if (player.positionx == x && player.positiony == y) {
						foundPlayer = true;
						break;
					}
				}
				if (!foundPlayer)
					return Vector2{ x, y };
			}
		}
	}
	return Vector2{1, 1};
}

//Updates the clients with a certain user
void SendUpdatedUser(PlayerInfo player) {
	// ID |	X | Y |
	string message = "";
	message.push_back(DATA_TYPE_UPDATE);
	message.append(to_string(player.id));
	message.push_back(DATA_BREAKER);
	message.append(to_string(player.positionx));
	message.push_back(DATA_BREAKER);
	message.append(to_string(player.positiony));
	message.push_back(DATA_END);

	for (int i = 0; i < players.size(); i++) {
		PlayerInfo user = players.at(i);
		if (send(user.client, message.c_str(), message.length(), 0))
			cout << "Message sending failed!" << endl;
	}
}

void HandleClientConnection(PlayerInfo player)
{
	//Update the clients
	SendUpdatedUser(player);

	char buffer[MAXRECVBUFFER];
	while (recv(player.client, buffer, MAXRECVBUFFER, 0) > 0)
	{
		
	}
	cout << "Player " << player.avatar << " disconnected!" << endl;
}

int main()
{
	//Variables
	SOCKET server;
	SOCKADDR_IN serverAddr;
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != NO_ERROR)
	{
		cout << "WSAStartup failed!" << endl;
		return 1;
	}

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (server == SOCKET_ERROR)
	{
		cout << "Socket failed!" << endl;
		return 2;
	}

	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3434);

	if (bind(server, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "Bind failed!" << endl;
		return 3;
	}

	if (listen(server, MAXPENDINGCONNECTIONS) == SOCKET_ERROR)
	{
		cout << "Listen failed!" << endl;
		return 4;
	}

	cout << "Server Started!" << endl;

	while (true)
	{
		SOCKET client;
		SOCKADDR_IN clientAddr;
		int clientLength = sizeof(clientAddr);
		client = accept(server, (struct sockaddr*)&clientAddr, &clientLength);
		if (client == INVALID_SOCKET)
		{
			cout << "accept failed!" << endl;
		}
		else
		{
			char ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(clientAddr.sin_addr), ip, INET_ADDRSTRLEN);
			cout << "Client " << ip << " connected!" << endl;
		
			PlayerInfo player;
			player.client = client;
			player.id = playerCount++;
			player.avatar = playerAvatars++;

			Vector2 emptyPosition = GetEmptySpot();
			player.positionx = emptyPosition.x;
			player.positiony = emptyPosition.y;

			players.push_back(player);
			DrawWorld();
			thread* clientThread = new thread(HandleClientConnection, player);
		}
	}
	
	//The server closed
	WSACleanup();
	return 0;
}

