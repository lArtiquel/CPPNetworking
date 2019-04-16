#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include <string>
#include <windows.h>
#include <vector>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

HANDLE fMutex;

struct Messages {
	char to[64];
	char from[64];
	char message[1024];
};

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
							string to;
							if (!getline(is, to, ' ')) {
								char end[64] { '!', 'e', 'n', 'd', '\0' };	// it should be size of from buffer
								send(sock, end, sizeof(end), NULL);	
								quit = true;
								break;
							}

							char from[64];
							if (!is.getline(from, sizeof(from), ' ')) {
								char end[64]{ '!', 'e', 'n', 'd', '\0' };
								send(sock, end, sizeof(end), NULL);
								quit = true;
								break;
							}

							char inMsg[1024];
							if (!is.getline(inMsg, sizeof(inMsg))) {
								char end[64]{ '!', 'e', 'n', 'd', '\0' };
								send(sock, end, sizeof(end), NULL);
								quit = true;
								break;
							}

							if (to == cTel) {
								send(sock, from, sizeof(from), NULL);
								send(sock, inMsg, sizeof(inMsg), NULL);
							}
						}
						if (!quit) {
							char end[64]{ '!', 'e', 'n', 'd', '\0' };
							send(sock, end, sizeof(end), NULL);
						}
						is.close();
					}
					else {
						char end[64]{ '!', 'e', 'n', 'd', '\0' };
						send(sock, end, sizeof(end), NULL);
						cout << "error open file!\n";
					}
					ReleaseMutex(fMutex);
					break;
				}

				case WAIT_ABANDONED:
					cout << "wait abandoned\n";
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
					// send and collect all "sended messages"
					vector<Messages> msgs;
					ifstream is("messages.txt");
					bool quit = false;
					if (is.is_open()) {
						while (!is.eof()) {
							char to[64];
							if (!is.getline(to, sizeof(to), ' ')) {
								char end[64]{ '!', 'e', 'n', 'd', '\0' };	
								send(sock, end, sizeof(end), NULL);
								quit = true;
								break;
							}

							string from;
							if (!getline(is, from, ' ')) {
								char end[64]{ '!', 'e', 'n', 'd', '\0' };
								send(sock, end, sizeof(end), NULL);
								quit = true;
								break;
							}

							char inMsg[1024];
							if (!is.getline(inMsg, sizeof(inMsg))) {
								char end[64]{ '!', 'e', 'n', 'd', '\0' };
								send(sock, end, sizeof(end), NULL);
								quit = true;
								break;
							}

							if (from == cTel) {
								send(sock, to, sizeof(to), NULL);
								send(sock, inMsg, sizeof(inMsg), NULL);
							}
							else {	// we just collect messages, not from this telephone
								Messages msg;
								memcpy(msg.to, to, sizeof(to));
								memcpy(msg.from, from.c_str(), sizeof(msg.from));
								memcpy(msg.message, inMsg, sizeof(inMsg));
								msgs.push_back(msg);
							}
						}
						if (!quit) {
							char end[64]{ '!', 'e', 'n', 'd', '\0' };
							send(sock, end, sizeof(end), NULL);
						}
						is.close();
					}
					else {
						char end[64]{ '!', 'e', 'n', 'd', '\0' };
						send(sock, end, sizeof(end), NULL);
						cout << "error open file!\n";
					}

					// and now simply rewrite file with telephone messages
					ofstream os("messages.txt", ios::trunc);
					for (int i = 0; i < msgs.size(); i++) {
						os << msgs[i].to << ' ' << msgs[i].from << ' ' << msgs[i].message;
						if (i != msgs.size() - 1) {
							os << '\n';
						}
					}
					os.close();

					ReleaseMutex(fMutex);
					break;
				}

				case WAIT_ABANDONED:
					cout << "wait abandoned\n";
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