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

void HandleServerConnection(SOCKET server) {

	char buffer[MAXRECVBUFFER];
	while (recv(server, buffer, MAXRECVBUFFER, 0) > 0)
	{
		cout << "Message: " << buffer << endl;
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

	DrawWorld();
	thread* serverThread = new thread(HandleServerConnection, server);

	string message = "";
	while (message != "exit")
	{
		getline(cin, message);
		if (send(server, message.c_str(), message.length() + 1, 0) == SOCKET_ERROR)
		{
			cout << "send failed!" << endl;
			return 4;
		}
		cout << "Message sent!" << endl;
	}
	if (closesocket(server) == SOCKET_ERROR)
	{
		cout << "closesocket failed!" << endl;
		return 5;
	}
	WSACleanup();
	return 0;
}
