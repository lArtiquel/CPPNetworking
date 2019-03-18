#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <ctime>
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
	// Create a SOCKET for listening for
	// incoming connection requests.
	SOCKET SrvSocket;
	SrvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (SrvSocket == INVALID_SOCKET) {
		wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = htonl(INADDR_ANY); 
	addrServer.sin_port = htons(20131);


	if (bind(SrvSocket, (SOCKADDR *)& addrServer, sizeof(addrServer)) == SOCKET_ERROR) {
		wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
		closesocket(SrvSocket);
		WSACleanup();
		return 1;
	}

	while (1)
	{
		struct sockaddr_in cliAddr;			//for incoming client addresses
		int cliAddrLen = sizeof(cliAddr);	//client address length
		char messageSize[4];				//incoming message size

		if (recvfrom(SrvSocket, messageSize, sizeof(int), 0, (struct sockaddr *)&cliAddr, &cliAddrLen) == INVALID_SOCKET)
		{
			cout << "socket is broken, we are closing\n";		//if socket broken then quit
			break;
		}

		int messageLengthINT;
		memcpy(&messageLengthINT, messageSize, sizeof(int));
		char *message = new char[messageLengthINT];
		int count;							//for number received bytes


		if ((count = recvfrom(SrvSocket, message, messageLengthINT, 0, (struct sockaddr *)&cliAddr, &cliAddrLen)) < messageLengthINT)
		{
			if (count == INVALID_SOCKET)
			{
				cout << "socket is broken, we are closing\n";		//if socket broken then quit
				delete message;
				break;
			}
			cout << "partial receive!\n";
		}

		if (messageLengthINT > 1)
		{
			for (int i = 0; i < messageLengthINT-1; i += 2)
			{
				swap(message[i], message[i + 1]);
			}
		}

		time_t t = time(0);   // get time now
		struct tm * now = localtime(&t);
		cout << (now->tm_year + 1900) << '.'
			<< (now->tm_mon + 1) << '.'
			<< now->tm_mday
			<< ' '
			<< now->tm_hour << ':'
			<< now->tm_min << ':'
			<< now->tm_sec
			<< " received message from IP " << inet_ntoa(cliAddr.sin_addr) <<'\n';
			

		if (sendto(SrvSocket, message, messageLengthINT, 0, (struct sockaddr *)&cliAddr, cliAddrLen) == INVALID_SOCKET)
		{
			cout << "socket is broken, we are closing\n";		//if socket broken then quit
			delete message;
			break;
		}

		cout << "sended response to him!" << '\n';
		delete message;
	}

	closesocket(SrvSocket);
	WSACleanup();
	system("pause");
	return 0;
}