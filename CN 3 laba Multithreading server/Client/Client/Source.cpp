#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <io.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

int main(int argc, char* argv[])
{
	//----------------------
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error: %ld\n", iResult);
		return 1;
	}

	//----------------------
	// Create a SOCKET for connecting to server
	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	addrServer.sin_port = htons(20131);

	//----------------------
	// Connect to server.
	iResult = connect(ConnectSocket, (SOCKADDR*)&addrServer, sizeof(addrServer));
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		printf("Unable to connect to server: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	while (1)
	{
		double estimatedCost;
		int count, currentPosition, receivedBytes, sendedBytes;
		char *costBuf = new char[sizeof(double)];
		cout << "please, enter estimated cost: ";
		cin >> estimatedCost; cout << '\n';

		memcpy(costBuf, &estimatedCost, sizeof(double));

		currentPosition = 0;
		count = sizeof(double);
		while (count > 0 && (sendedBytes = send(ConnectSocket, costBuf + currentPosition, count, 0)) != INVALID_SOCKET)
		{
			currentPosition += sendedBytes;
			count -= sendedBytes;
		}
		delete costBuf;
		if (sendedBytes == INVALID_SOCKET)
		{
			cout << "socket broken on sending\n";
			break;
		}

		//now receive response from server
		int size;
		char *sizeBuf = new char[sizeof(int)];

		currentPosition = 0;
		count = sizeof(int);
		while (count > 0 && (receivedBytes = recv(ConnectSocket, sizeBuf + currentPosition, count, 0)) != INVALID_SOCKET)
		{
			currentPosition += receivedBytes;
			count -= receivedBytes;
		}
		if (receivedBytes == INVALID_SOCKET)
		{
			cout << "socket broken on receiving response size \n";
			delete sizeBuf;
			break;
		}
		memcpy(&size, sizeBuf, sizeof(int));
		delete sizeBuf;

		char *response = new char[size];
		currentPosition = 0;
		count = size;
		while (count > 0 && (receivedBytes = recv(ConnectSocket, response + currentPosition, count, 0)) != INVALID_SOCKET)
		{
			currentPosition += receivedBytes;
			count -= receivedBytes;
		}
		if (receivedBytes == INVALID_SOCKET)
		{
			cout << "socket broken on receiving response \n";
			delete response;
			break;
		}

		cout << "suggested tours for u:\n " << response << '\n';
		delete response;

		int choise;
		cout << "do you want to continue?(1 - yes, else - no): ";
		if (cin >> choise)
		{
			if (choise != 1) break;
		}
		else break;
	}

	closesocket(ConnectSocket);
	WSACleanup();
	system("pause");
	return 0;
}

