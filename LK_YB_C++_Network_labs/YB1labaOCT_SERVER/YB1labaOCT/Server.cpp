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
		char a[200], b[200];
		recv(newConnection, a, sizeof(b), 0);
		if (strcmp(a, "exit") == 0) break;
		recv(newConnection, b, sizeof(b), 0);
		if (strcmp(b, "exit") == 0) break;
		char flag[30];
		
		if (strcmp(a, b) == 0)
		{
			cout << "Одинаковые строки" << endl;
			strcpy_s(flag, "одинаковые строки");
		}
		else
		{
			cout << "не одинаковые строки" << endl;
			strcpy_s(flag, "не одинаковые строки");
		}

		send(newConnection, flag, sizeof(flag), 0);
	}
	
	closesocket(newConnection);
	closesocket(sListen);
	WSACleanup();
	system("pause");
	return 0;
}
