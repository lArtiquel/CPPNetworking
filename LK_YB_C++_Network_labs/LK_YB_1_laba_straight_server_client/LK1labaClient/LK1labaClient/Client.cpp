#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <algorithm>
#include <string>

using namespace std;

bool is_number(const string& s)
{
	if (s.empty())
		return false;

	for (int i = 0; i < s.size(); i++)
	{
		switch (s[i])
		{
			case '0': break;
			case '1': break;
			case '2': break;
			case '3': break;
			case '4': break;
			case '5': break;
			case '6': break;
			case '7': break;
			case '8': break;
			case '9': break;
			default: return false;
		}
 	}
	return true;
}

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
		string num;
		bool isCorrect;

		do {
			isCorrect = true;
			cout << "¬ведите две строки('987654' - to quit):" << endl;
			cin >> num;

			if (!is_number(num) || num.size() != 6) {
				cout << "irregular input\n";
				isCorrect = false;
			}
		} while (!isCorrect);

		int number = stoi(num, nullptr, 10);
		char numBuf[5];
		memcpy(numBuf, &number, sizeof(int));
		numBuf[4] = '\0';

		send(s, numBuf, sizeof(numBuf), 0);
		
		if (number == 987654) break;

		char recvBuf[30];

		recv(s, recvBuf, sizeof(recvBuf), 0);

		cout << recvBuf << endl;
	}

	closesocket(s);
	WSACleanup();
	system("pause");
	return 0;
}
