#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
// link ws2_32 in prj settings!
using namespace std;

int main()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		cout <<"WSAStartup failed with error\n";
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		cout << "Error at socket\n";
		WSACleanup();
		return 2;
	}

	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	addrServer.sin_port = htons(50001);

	iResult = connect(sock, (SOCKADDR*)&addrServer, sizeof(addrServer));
	if (iResult == SOCKET_ERROR) {
		closesocket(sock);
		cout << "Unable to connect to server\n";
		WSACleanup();
		return 3;
	}

	while (1)
	{
		char outcomingString[64];
		cout << "enter another string: ";
		cin.getline(outcomingString, sizeof(outcomingString));

		send(sock, outcomingString, sizeof(outcomingString), NULL);
		if (strcmp(outcomingString, "!quit") == 0) {
			break;
		}

		int length = 0;
		recv(sock, reinterpret_cast<char*>(&length), sizeof(int), NULL);

		cout << "server sended: " << length << " - length of sended string\n";
		if (length > 15) {
			char incomingString[64];
			recv(sock, incomingString, sizeof(incomingString), NULL);
			cout << "another server's response: \"";
			cout << incomingString << "\" --- substring before space\n";
		}	
	}

	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}