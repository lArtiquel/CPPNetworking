#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include <string>
#include <windows.h>

using namespace std;

HANDLE fMutex;

DWORD WINAPI Payload(
	LPVOID lpParameter
)
{
	SOCKET sock = (SOCKET)lpParameter;

	char *clientTelBuf = new char[64];
	recv(sock, clientTelBuf, 64, NULL);
	string cTel(clientTelBuf);
	delete clientTelBuf;
	
	while (true) {
		int cmd;
		recv(sock, reinterpret_cast<char*>(&cmd), sizeof(int), NULL);

		switch (cmd) {
			case 0: 
				cout << "successfully disconnected!\n";
				closesocket(sock); 
				return 0;
			case 1:
				{
					char tel[64];
					recv(sock, tel, sizeof(tel), NULL);
					char message[1024];
					recv(sock, message, sizeof(message), NULL);

					DWORD dwWaitResult = WaitForSingleObject(
						fMutex, 
						INFINITE);  

					switch (dwWaitResult)
					{
						case WAIT_OBJECT_0:
						{
							ofstream os("messages.txt", ios::app);
							if (os.is_open())
							{
								os << tel << ' ' << message << '\n';
								os.close();
							}
							ReleaseMutex(fMutex);
							break;
						}
							
						case WAIT_ABANDONED:
							return 1;
					}
					break;
				}
				
			case 2: 
				{
					DWORD dwWaitResult = WaitForSingleObject(
						fMutex,
						INFINITE);

					switch (dwWaitResult)
					{
						case WAIT_OBJECT_0:
						{
							ifstream is("messages.txt");
							bool quit = false;
							if (is.is_open()) {
								while (!is.eof()) {
									string tel;
									if (!getline(is, tel, ' ')) {
										send(sock, "!end", sizeof("!end"), NULL);
										quit = true;
										break;
									}

									char inMsg[1024];
									if (!is.getline(inMsg, sizeof(inMsg))) {
										send(sock, "!end", sizeof("!end"), NULL);
										quit = true;
										break;
									}

									if (tel == cTel) {
										send(sock, inMsg, sizeof(inMsg), NULL);
									}
								}
								if (!quit) {
									send(sock, "!end", sizeof("!end"), NULL);
								}
								is.close();
							}
							else {
								send(sock, "!end", sizeof("!end"), NULL);
								cout << "error open file!\n";
							}
							ReleaseMutex(fMutex);
							break;
						}

						case WAIT_ABANDONED:
							return 1;
					}

					break;
				}
				
			default: 
				cout << "some error!\n";
				closesocket(sock);
				return 0;
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

	fMutex = CreateMutex(NULL, FALSE, NULL);	// create file mutex with default security attribs, not owned, unnamed

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