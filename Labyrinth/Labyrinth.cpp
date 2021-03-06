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

//player controls
const char MOVE_UP = 'w';
const char MOVE_RIGHT = 'd';
const char MOVE_LEFT = 'a';
const char MOVE_DOWN = 's';

//datatypes
const char DATA_TYPE_START = (char)196;
const char DATA_TYPE_UPDATE = (char)197;
const char DATA_TYPE_END = (char)198;

const char DATA_END = (char)199;
const char DATA_USER_BREAK = (char)200;
const char DATA_BREAKER = (char)201;

const char WALL_CHAR = (char)178;
const char EMPTY_SPOT = ' ';

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
					if (player.positionx == y && player.positiony == x) {
						cout << player.avatar;
						foundPlayer = true;
						break;
					}
				}
				if (!foundPlayer)
					cout << EMPTY_SPOT;
			}
		}
		cout << endl;
	}
}

Vector2 GetEmptySpot() {
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {
			char worldSpot = WORLD_MAP[y][x];
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
void SendUpdatedUser(PlayerInfo player, bool sendToUser) {
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

		if (!sendToUser && user.client == player.client)
			continue;

		if (send(user.client, message.c_str(), message.length(), 0) == SOCKET_ERROR)
			cout << "Message sending failed!" << endl;
	}
}

//Sends all the current users data
void SendWholeMap(PlayerInfo player) {
	//ID | avatar | X | Y 
	string message = "";
	message.push_back(DATA_TYPE_START);

	for (int i = 0; i < players.size(); i++) {
		PlayerInfo user = players.at(i);

		message.append(to_string(user.id));
		message.push_back(DATA_BREAKER);
		message.push_back(user.avatar);
		message.push_back(DATA_BREAKER);
		message.append(to_string(user.positionx));
		message.push_back(DATA_BREAKER);
		message.append(to_string(user.positiony));
		message.push_back(DATA_USER_BREAK);
	}

	message.push_back(DATA_END);

	if (send(player.client, message.c_str(), message.length(), 0) == SOCKET_ERROR)
		cout << "Map sending failed!" << endl;
}

void RemoveUser(PlayerInfo player) {

	for (int index = 0; index < players.size(); index++) {
		PlayerInfo user = players.at(index);

		if (user.client == player.client) {
			players.erase(players.begin() + index);
			break;
		}

	}

	//Update the clients
	for (int i = 0; i < players.size(); i++) {
		SendWholeMap(players.at(i));
	}

	//Rebuild the map in the server
	DrawWorld();
}

void MovePlayer(int playerID, char direction) {

	PlayerInfo player;
	bool foundplayer = false;
	for (int i = 0; i < players.size(); i++) {
		PlayerInfo& user = players.at(i);
		if (user.id == playerID) {
			player = user;
			foundplayer = true;
			break;
		}
	}

	if (!foundplayer)
		return;

	Vector2 predictedPosition{player.positionx, player.positiony};

	switch (direction) {
		case MOVE_UP:
			predictedPosition.y--;
			break;
		case MOVE_RIGHT:
			predictedPosition.x++;
			break;
		case MOVE_LEFT:
			predictedPosition.x--;
			break;
		case MOVE_DOWN:
			predictedPosition.y++;
			break;
		default:
			return;
	}

	bool isEmpty = false;
	bool hasPlayer = false;

	if (WORLD_MAP[predictedPosition.y][predictedPosition.x] == EMPTY_SPOT)
		isEmpty = true;
	

	for (int i = 0; i < players.size(); i++) {
		PlayerInfo user = players.at(i);
		if (user.positionx == predictedPosition.x && user.positiony == predictedPosition.y) {
			cout << user.positionx << "," << user.positiony << endl;
			cout << "There's a player here!" << endl;
			hasPlayer = true;
			break;
		}
	}

	if (isEmpty && !hasPlayer) {
		player.positiony = predictedPosition.y;

		//Update player data
		for (int i = 0; i < players.size(); i++) {
			PlayerInfo& user = players.at(i);
			if (user.id == player.id) {
				user.positionx = predictedPosition.x;
				user.positiony = predictedPosition.y;
				break;
			}
		}
		DrawWorld();

		//Update the clients
		for (int i = 0; i < players.size(); i++) {
			SendWholeMap(players.at(i));
		}
	}
}

void HandleClientConnection(PlayerInfo player)
{
	//Update the clients
	for (int i = 0; i < players.size(); i++) {
		SendWholeMap(players.at(i));
	}

	char buffer[MAXRECVBUFFER];
	while (recv(player.client, buffer, MAXRECVBUFFER, 0) > 0)
	{
		MovePlayer(player.id, buffer[0]);
	}

	
	RemoveUser(player);

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

