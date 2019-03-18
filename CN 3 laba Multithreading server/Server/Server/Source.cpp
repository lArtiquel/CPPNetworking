#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

using namespace std;
struct travelOffers				//travelOffers struct
{
	string name;
	string  cost;
	string duration;
	string type;
};

vector<travelOffers> v;			//to keep data in RAM

bool readDataFromDocument()
{
	ifstream file("travelOffers.txt");
	if (!file.is_open())
		return false;
	file.seekg(0, file.beg);
	vector<travelOffers> temp;
	bool isCool = true;
	while (!file.eof())
	{
		travelOffers t;
		if (!getline(file, t.name))
			isCool = false;
		if (!getline(file, t.cost))
			isCool = false;
		if (!getline(file, t.duration))
			isCool = false;
		if (!getline(file, t.type))
			isCool = false;

		cout << t.name << ' ' << t.cost << ' ' << t.duration << ' ' << t.type << '\n';
		if (isCool)
			temp.push_back(t);
		else
		{
			cout << "file  not readed exellent! number of tours readed:" << temp.size() << '\n';
			file.close();
			return false;
		}
	}
	v.clear();
	v = temp;
	cout << "file readed exellent! number of tours:" << v.size() << '\n';
	file.close();
	return true;
}

DWORD WINAPI ThreadProc(
	__in  LPVOID lpParameter
)
{
	SOCKET AcceptSocket = (SOCKET)lpParameter;	
	
	char recvBuf[sizeof(double)];
	while (1) {
		int count, bytesReceived, currentPosition;

		count = sizeof(double);
		currentPosition = 0;
		while (count > 0 && (bytesReceived = recv(AcceptSocket, recvBuf + currentPosition, count, 0)) != INVALID_SOCKET)
		{
			currentPosition += bytesReceived;
			count -= bytesReceived;
		}
		if (bytesReceived == INVALID_SOCKET) break;

		double costRecv;
		memcpy(&costRecv, recvBuf, sizeof(double));	//convert double array to double number
		readDataFromDocument();						//update  info
		string response;

		for (int i = 0; i < v.size(); i++)
		{
			if (costRecv <= stod(v.at(i).cost))
			{
				response += "name: " + v.at(i).name +
					"; cost: " + v.at(i).cost + 
					"; duration: " + v.at(i).duration +
					"; type: " + v.at(i).type + '\n';
			}
		}

		if (response.empty()) response = "Nothing to suggest!\n";

		int responseSize = response.size() + 1;
		char *responseSizeBuf = new char[sizeof(int)];		
		memcpy(responseSizeBuf, &responseSize, sizeof(int));					//convert int to int array

		int bytesSend;
		count = sizeof(int);													//send response size to client
		currentPosition = 0;
		while (count > 0 && (bytesSend = send(AcceptSocket, responseSizeBuf + currentPosition, count, 0)) != INVALID_SOCKET)
		{
			currentPosition += bytesSend;
			count -= bytesSend;
		}
		delete responseSizeBuf;
		if (bytesSend == INVALID_SOCKET) break;

		char *responseBuf = new char[responseSize];
		memcpy(responseBuf, response.c_str(), response.size());						//convert string to byte array
		responseBuf[responseSize - 1] = '\0';

		count = responseSize;													//send response to client
		currentPosition = 0;
		while (count > 0 && (bytesSend = send(AcceptSocket, responseBuf + currentPosition, count, 0)) != INVALID_SOCKET)
		{
			currentPosition += bytesSend;
			count -= bytesSend;
		}
		delete responseBuf;
		if (bytesSend == INVALID_SOCKET) break;

		printf("received request from %d\n", AcceptSocket);
	}
	cout << "thread termiating!\n";
	closesocket(AcceptSocket);
	return 0;
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
	addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
	addrServer.sin_port = htons(20131);


	if (bind(ListenSocket, (SOCKADDR *)& addrServer, sizeof(addrServer)) == SOCKET_ERROR) {
		wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);
	vector<HANDLE> hThreadArray;
	readDataFromDocument();						//update  info

	while (1)
	{
		SOCKET AcceptSocket = accept(ListenSocket, (SOCKADDR*)&addrClient, &len);
		if (AcceptSocket == INVALID_SOCKET) break; 

		DWORD dwThread;
		HANDLE hThread = CreateThread(NULL,							//security flag
										0,							//stack size
										ThreadProc,					//thread function
										(LPVOID)AcceptSocket,		//function parameter LPTHREAD_START_ROUTINE
										0,							//creation flags CREATE_SUSPENDED - to stop execution function at start/ ExitThread/SuspendThread
										&dwThread);					//thread ID
		if (hThread == NULL)										
		{
			closesocket(AcceptSocket);
			wprintf(L"Thread Creat Failed!\n");
			break;
		}
		else
			hThreadArray.push_back(hThread);						//keep desc and id thread to the end
	}

	WaitForMultipleObjects(hThreadArray.size(), &hThreadArray.at(0), TRUE, INFINITE);	// Wait until all threads have terminated.

	for (int i = 0; i < hThreadArray.size(); i++)
	{
		CloseHandle(hThreadArray[i]);								// Close all thread handles and free memory allocations.
	}

	cout << "program ending!";
	closesocket(ListenSocket);
	WSACleanup();
	system("pause");
	return 0;
}

