#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		cout << "WSAStartup failed with error\n";
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		cout << "Error at socket\n";
		WSACleanup();
		return 2;
	}

	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	addrServer.sin_port = htons(50001);

	iResult = connect(sock, (SOCKADDR*)&addrServer, sizeof(addrServer));
	if (iResult == SOCKET_ERROR) {
		closesocket(sock);
		cout << "Unable to connect to server\n";
		WSACleanup();
		return 3;
	}

	char cTel[64];
	cout << "Welcome to the Post Client\n";
	cout << "please, enter your phone: ";
	cin >> cTel;
	send(sock, cTel, sizeof(cTel), NULL);

	while (true)
	{
		cout << "enter command:\n"
			<< "0 - quit\n"
			<< "1 - view incoming messages\n"
			<< "2 - delete all sended messages\n";

		int choise;
		bool quit = false;
		if (!(cin >> choise)) {
			cout << "irregular input!\nTry agian!\n";
		}
		else {
			send(sock, reinterpret_cast<char*>(&choise), sizeof(int), NULL);
			switch (choise)
			{
			case 0:
			{
				quit = true;
				break;
			}

			case 1:
			{
				char message[1024];
				char fromWhom[64];

				cout << "incoming messages:\n";
				int i = 1;
				while (true)
				{
					recv(sock, fromWhom, sizeof(fromWhom), NULL);
					if (!strncmp(fromWhom, "!end", sizeof("!end"))) {
						if (i == 1) {
							cout << "no incoming messages!\n";
						}
						break;
					}
					else {
						recv(sock, message, sizeof(message), NULL);
						
						cout << "FROM: " << fromWhom << '\n';
						cout << "message #" << i << ":\n";
						cout << message << '\n';
						i++;
					}					
				}
				break;
			}

			case 2:
			{
				char message[1024];
				char toWhom[64];

				cout << "sended messages:\n";
				int i = 1;
				while (true)
				{
					recv(sock, toWhom, sizeof(toWhom), NULL);
					if (!strncmp(toWhom, "!end", sizeof("!end"))) {
						if (i == 1) {
							cout << "no incoming messages!\n";
						}
						break;
					}
					else {
						recv(sock, message, sizeof(message), NULL);

						cout << "TO: " << toWhom << '\n';
						cout << "message #" << i << ":\n";
						cout << message << '\n';
						i++;
					}
				}

				cout << "all sended messages has been deleted!\n";
				break;
			}

			default:
			{
				quit = true;
				cout << "wrong cmd!\n";
			}
			}

			if (quit) break;
		}
	}

	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}