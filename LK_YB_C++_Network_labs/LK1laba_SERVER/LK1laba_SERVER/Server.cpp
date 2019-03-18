#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>
#include <string>
#include <Windows.h>

using namespace std;

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "rus");
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 2);
	WSAStartup(DllVersion, &wsaData);

	SOCKADDR_IN addr;
	int addrlen = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1280);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, 0);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);
	SOCKET newConnection;
	newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen);
	while (true)
	{
		char numBuf[5];
		recv(newConnection, numBuf, sizeof(numBuf), 0);

		int number, first, second;
		memcpy(&number, numBuf, sizeof(int));
		if (number == 987654) break;
		 
		first = ((number / 1000) % 10) + (((number / 1000) % 100) / 10) + ((number / 1000) / 100);
		second = ((number % 1000) / 100) + ((number % 1000) % 100 / 10) + ((number % 1000) % 10);

		char response[30];
		if (first == second)
		{
			cout << "Lucky" << endl;
			strcpy_s(response, "Lucky\n");
		}
		else
		{
			cout << "Not lucky" << endl;
			strcpy_s(response, "Not lucky\n");
		}

		send(newConnection, response, sizeof(response), 0);
	}

	closesocket(newConnection);
	closesocket(sListen);
	WSACleanup();
	system("pause");
	return 0;
}
