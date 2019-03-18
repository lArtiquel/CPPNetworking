#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <io.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

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
    // Create a SOCKET for connecting to server
    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return 1;
    }
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    sockaddr_in addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    addrServer.sin_port = htons(20131);

	//----------------------
    // Connect to server.
    iResult = connect( ConnectSocket, (SOCKADDR*) &addrServer, sizeof(addrServer) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (ConnectSocket);
        printf("Unable to connect to server: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
	
	while(1)
	{
		//input coordinates
		int32_t x, y;
		cout << "please, enter x coordinate: ";
		cin >> x; cout << '\n';
		cout << "please, enter y coordinate: ";
		cin >> y; cout << '\n';
		//serialize x and y
		char bufX[4], bufY[4];
		SerializeInt32(bufX, x);
		SerializeInt32(bufY, y);
		//send x y to server
		send(ConnectSocket, bufX, 4, 0);
		send(ConnectSocket, bufY, 4, 0);
		
		//now receive response from server
		int count;
		char response[10];
		count = recv(ConnectSocket, response, 10, 0);
		if (count == 0) break;
		if (count == SOCKET_ERROR) break;//broken socket
		response[count-1] = '\0';
		cout << "received message from server: " << response << '\n';
	}
	
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}

