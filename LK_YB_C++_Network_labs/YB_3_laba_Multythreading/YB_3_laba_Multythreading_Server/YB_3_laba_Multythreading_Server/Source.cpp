#include <iostream>
#include <WinSock2.h>
// do not forget to link Winsock lib in prj settings
using namespace std;

int checkLength(const char* buf, int size) {
	int length = 0;
	for (int i = 0; i < size; i++) {
		if (buf[i] == '\0') {
			return length;
		}
		else
			length++;
	}
	return -1;
}

DWORD WINAPI Payload(
	LPVOID lpParameter
)
{
	SOCKET sock = (SOCKET)lpParameter;

	while (true) {
		char str[64];
		recv(sock, str, sizeof(str), NULL);
		if (strcmp(str, "!quit") == 0) {
			cout << "done with another client!\n";
			return 0;
		}

		int length = checkLength(str, sizeof(str));

		send(sock, reinterpret_cast<char*>(&length), sizeof(int), NULL);

		if (length % 2) {
			for (int i = length / 2; i < length; i++) {		// yep, allocate this symbol after terminator
				str[i] = str[i + 1];
			}

			send(sock, str, sizeof(str), NULL);
		}
	}
}

int main() {
	WORD wVersionRequested;
	WSADATA wsadata;
	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsadata) != NO_ERROR) {
		cout << "init error\n";
		return 1;
	}

	SOCKET ls = socket(AF_INET, SOCK_STREAM, 0);
	if (ls == INVALID_SOCKET) {
		cout << "invalid socket\n";
		WSACleanup();
		return 2;
	}

	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(50001);
	local_addr.sin_addr.s_addr = 0;

	if (bind(ls, (sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
		cout << "bind err\n";
		closesocket(ls);
		WSACleanup();
		return 3;
	}

	if (listen(ls, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen error\n";
		closesocket(ls);
		WSACleanup();
		return 4;
	}

	SOCKADDR_IN addrClient;
	int addrlen = sizeof(SOCKADDR);

	while (true) {
		SOCKET s = accept(ls, (SOCKADDR*)&addrClient, &addrlen);
		if (s == INVALID_SOCKET) break;

		DWORD dwThread;
		HANDLE hThread = CreateThread(NULL,
			0,
			Payload,
			(LPVOID)s,
			0,
			&dwThread);

		if (hThread == NULL) {
			cout << "thread creation failed!\n";
			closesocket(s);
			Sleep(60000);			// server is tired:)
		}
		else {
			cout << "new client successfully connected!\n";
			CloseHandle(hThread);	// we do not need it anymore
		}
	}

	closesocket(ls);
	WSACleanup();
	return 0;
}