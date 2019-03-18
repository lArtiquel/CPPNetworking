#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <Windows.h>

using namespace std;

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "rus");
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	struct sockaddr_in addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1280);
	addr.sin_family = AF_INET;

	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	connect(s, (struct sockaddr*) &addr, sizeof(addr));

	while (true)
	{
		char a[200], b[200], c[30];
		cout << "¬ведите две строки(exit - to quit):" << endl;

		cin >> a;
		cin >> b;

		send(s, a, sizeof(a), 0);
		if (strcmp(a, "exit") == 0) break;
		send(s, b, sizeof(b), 0);
		if (strcmp(b, "exit") == 0) break;
		recv(s, c, sizeof(c), 0);
		cout << c << endl;
	}
	
	closesocket(s);
	WSACleanup();
	system("pause");
	return 0;
}
