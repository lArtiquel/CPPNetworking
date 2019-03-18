#define _WINSOCK_DEPCRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <io.h>
#include <iostream>
#include <string>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

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
	SOCKET udpEchoSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpEchoSocket == INVALID_SOCKET) {
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

	int srvAddrLen = sizeof(addrServer);

	while (1)
	{
		string buf;															//message variable
		cout << "enter message to send to server( press 'quit' to quit)\n";
		getline(cin, buf);													//read input string
		if (buf == "quit") break;											//to break the loop
		
		int size = buf.size();												//size of string
		char sizeBuf[4];													//create size buffer
		memcpy(sizeBuf, &size, sizeof(int));								//fill sizebuf size'om
		//send to server the number of bytes to receive
		if (sendto(udpEchoSocket, sizeBuf, sizeof(int), 0, (struct sockaddr *)&addrServer, srvAddrLen) == INVALID_SOCKET)
		{
			cout << "socket is broken, we are closing\n";		//if socket broken then quit
			break;
		}
		//send buffer to server
		if (sendto(udpEchoSocket, buf.c_str(), size, 0, (struct sockaddr *)&addrServer, srvAddrLen) == INVALID_SOCKET)
		{
			cout << "socket is broken, we are closing\n";		//if socket broken then quit
			break;
		}	
		
		int count;
		char *response = new char[size+1];						//cause sending without \0

		if ((count = recvfrom(udpEchoSocket, response, size, 0, (struct sockaddr *)&addrServer, &srvAddrLen)) < size)
		{
			if (count == INVALID_SOCKET)
			{
				cout << "socket is broken, we are closing\n";		//if socket broken then quit
				delete response;
				break;
			}
			cout << "partial send from server."<< "\n";
		}
		
		response[size] = '\0';
		cout << "received response from server: " << response <<"\n";
		delete response;
		
		cout << "do u want to continue(1-yes, else-no): ";
		int choise;
		if (cin >> choise)
		{
			if (choise != 1) break;
		}
			else break;
	}
	
	closesocket(udpEchoSocket);									//close socket
	WSACleanup();												//call clean up function
	system("pause");
	return 0;
}

