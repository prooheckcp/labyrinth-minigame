//Includes||
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <thread>
//________||

//Defines||
#define MAXRECVBUFFER 1024
//_______||

//Namespaces||
using namespace std;
//__________||

//Constants||
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

//Structures||
struct PlayerInfo {
	int id;
	char avatar;
	int positionx;
	int positiony;
};

struct Vector2 {
	int x;
	int y;
};
//__________||

//Variables||
vector<PlayerInfo> players;
int playerID = -1;
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
//Parsers
void LoadUsers(char buffer[]) {
	
	//Cleaning the players socket
	players.clear();

	PlayerInfo nextUser;
	string userID = "";
	string userPosX = "";
	string userPosY = "";

	int currentArgument = 0;
	for (int i = 1; i < MAXRECVBUFFER; i++) {
		char currentCharacter = buffer[i];

		if (currentCharacter == DATA_USER_BREAK) {

			cout << userPosY << endl;

			nextUser.id = stoi(userID);
			nextUser.positionx = stoi(userPosX);
			nextUser.positiony = stoi(userPosY);
			players.push_back(nextUser);
			currentArgument = 0;
			userID = "";
			userPosX = "";
			userPosY = "";
			nextUser = PlayerInfo();
			continue;
		}
		else if (currentCharacter == DATA_BREAKER) {
			currentArgument++;
			continue;
		}
		else if (currentCharacter == DATA_END) {
			break;
		}

		if (currentArgument == 0) 
			userID.push_back(currentCharacter); //ID
		else if (currentArgument == 1) 
			nextUser.avatar = currentCharacter; //AVATAR
		else if (currentArgument == 2)
			userPosX.push_back(currentCharacter); //X
		else if (currentArgument == 3) 
			userPosY.push_back(currentCharacter); //Y

	}
}

void UpdateUser(char buffer[]) {

	PlayerInfo updatedUser;
	int currentArgument = 0;
	for (int i = 1; i < MAXRECVBUFFER; i++) {
		char currentCharacter = buffer[i];

		if (currentCharacter == DATA_BREAKER) {
			currentArgument++;
			continue;
		}
		else if (currentCharacter == DATA_END) {
			break;
		}

		if (currentArgument == 0) {
			//ID
			updatedUser.id = currentCharacter - '0';
		}
		else if (currentArgument == 1) {
			//X
			updatedUser.positionx = currentCharacter;
		}
		else if (currentArgument == 2) {
			//Y
			updatedUser.positiony = currentCharacter - '0';
		}
	}

	bool foundUser = false;
	for (int i = 0; i < players.size(); i++) {
		PlayerInfo user = players.at(i);
		if (updatedUser.id == user.id) {
			user.positionx = updatedUser.positionx;
			user.positiony = updatedUser.positiony;
			foundUser = true;
			break;
		}
	}



}

void HandleServerConnection(SOCKET server) {

	char buffer[MAXRECVBUFFER];
	while (recv(server, buffer, MAXRECVBUFFER, 0) > 0)
	{
		char dataType = buffer[0];
		switch (dataType) {
			case DATA_TYPE_UPDATE:
				UpdateUser(buffer);
				break;
			case DATA_TYPE_START:
				LoadUsers(buffer);
				break;
		}
		DrawWorld();
	}
	cout << "Server commited seppuku" << endl;
}

int main()
{
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
		cout << "socket failed!" << endl;
		return 2;
	}

	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr.s_addr);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3434);

	if (connect(server, (struct sockaddr*)&serverAddr,
		sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "connect failed!" << endl;
		return 3;
	}

	cout << "Connected to Server!" << endl;

	thread* serverThread = new thread(HandleServerConnection, server);

	string message = "";
	while (message != "exit")
	{
		getline(cin, message);
		if (send(server, message.c_str(), message.length(), 0) == SOCKET_ERROR)
			cout << "Failed to send command!" << endl;
	}
	if (closesocket(server) == SOCKET_ERROR)
	{
		cout << "closesocket failed!" << endl;
		return 5;
	}
	WSACleanup();
	return 0;
}
