#define _WINSOCK_DEPCRECATED
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <ctime>

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

void SerializeInt32(char(&buf)[4], int32_t val)
{
	std::memcpy(buf, &val, 4);
}

int32_t ParseInt32(const char(&buf)[4])
{
	int32_t val;
	std::memcpy(&val, buf, 4);
	return val;
}

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
    // Create a SOCKET for listening for
    // incoming connection requests.
    SOCKET ListenSocket;
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    sockaddr_in addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY); //实际上是0
    addrServer.sin_port = htons(20131);


    if (bind(ListenSocket,(SOCKADDR *) & addrServer, sizeof (addrServer)) == SOCKET_ERROR) {
        wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

	//----------------------
    // Listen for incoming connection requests.
    // on the created socket
    if (listen(ListenSocket, 1) == SOCKET_ERROR) {
        wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

	SOCKADDR_IN addrClient;
	int len=sizeof(SOCKADDR);

	while(1)
	{
		SOCKET AcceptSocket=accept(ListenSocket,(SOCKADDR*)&addrClient,&len);
		if(AcceptSocket  == INVALID_SOCKET) break; 
		
		while (1) {
			SOCKET AcceptSocket = accept(ListenSocket, (SOCKADDR*)&addrClient, &len);
			if (AcceptSocket == INVALID_SOCKET) break;

			while (1) {
				char recvBufX[4], recvBufY[4];		//first quarter
				//receive information
				int countX = recv(AcceptSocket, recvBufX, 4, NULL);
				int countY = recv(AcceptSocket, recvBufY, 4, NULL);
				if (countX == 0 || countY == 0) break; //unseccessfully deliver
				if (countX == SOCKET_ERROR || countY == SOCKET_ERROR) break;	//connection broken
				
				//parse data
				int32_t x = ParseInt32(recvBufX);
				int32_t y = ParseInt32(recvBufY);
				//now find out which quarter
				const int responseLength = 10;
				char response[responseLength] = "  quarter";
				if (x > 0)
				{
					if (y > 0)
					{
						response[0] = '1';
					}
					else
					{
						response[0] = '4';
					}
				}
				else
				{
					if (y > 0)
					{
						response[0] = '2';
					}
					else
					{
						response[0] = '3';
					}
				}
				//send response to client(handled partial send)
				int sendCount = send(AcceptSocket, response, responseLength, NULL);
				
				if (sendCount == SOCKET_ERROR) break;
				//some log on cls
				time_t t = time(0);   // get time now
				struct tm * now = localtime(&t);
				cout << (now->tm_year + 1900) << '.'
					<< (now->tm_mon + 1) << '.'
					<< now->tm_mday
					<< ' '
					<< now->tm_hour << ':'
					<< now->tm_min << ':'
					<< now->tm_sec
					<< " received coordinate x = " << x << " and y = " << y
					<< ". And sended response: " << response << '\n';
			}
			closesocket(AcceptSocket);
		}
	}

	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}

