#include <iostream>
#include <WinSock2.h>
#include <thread>         // std::thread
#include <vector>
//#include "dbfunctions.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

using namespace std;



void procClient(SOCKET&);
bool getToken(const SOCKET&);
bool signIn(const SOCKET&);
bool checkIncomingMessages(const SOCKET&);
bool deleteIncomingMessage(const SOCKET&);
bool signOut(const SOCKET& serv);
int sendAll(const SOCKET&, const char *, int, int);
int recvAll(const SOCKET&, char*, int, int);
enum class commands {SIGNIN,      // all command what server can process
                     CHECKINCOMINGMESSAGES,
                     DELETEINCOMINGMESSAGE,
                     SIGNOUT};    // using class not to clutter up main namespace

int main()
{
//    //----------------------
//    // Create DataBase object and try to connect to DataBase
//    DataBase db;
//    bool onDBConnceted = db.connectToDataBase();
//    if(!onDBConnected){
//        cout<< "error to connect to DataBase!\n";
//        return -1;
//    }
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
    SOCKET listenSocket;
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
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


    if (::bind(listenSocket, (SOCKADDR *)& addrServer, sizeof(addrServer)) == SOCKET_ERROR) {
        wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
        ::closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    //----------------------
    // Listen for incoming connection requests.
    // on the created socket
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
        ::closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN addrClient;
    int lenAddrClient = sizeof(SOCKADDR);
    vector<thread> threadArr;

    while (1)
    {
        SOCKET acceptSocket = accept(listenSocket, (SOCKADDR*)&addrClient, &lenAddrClient);
        if (acceptSocket == INVALID_SOCKET) break;

        threadArr.push_back(thread(procClient, std::ref(acceptSocket)));
    }

    cout << "accept loop crashed! wait clients to end work...\n";
    for (int i = 0; i < threadArr.size(); i++){
        threadArr.at(i).join();
    }

    cout << "program ends!";
    ::closesocket(listenSocket);
    WSACleanup();
    system("pause");
    return 0;
}


void procClient(SOCKET& serv)
{
    cout<<"new thread created to process client: "<< this_thread::get_id()<< '\n';
    while(true)
    {
        // receving command to process
       char bufCommand[sizeof(int)];
       int command;
       recvAll(serv, bufCommand, sizeof(int), 0);
       // converting from char buffer to int number
       memcpy(&command, bufCommand, sizeof(int));
       // check what command we received
       switch(command)
       {
            case commands::SIGNIN: if(signIn(serv)) break; else return;
            case commands::CHECKINCOMINGMESSAGES: if(checkIncomingMessages(serv)) break; else return;
            case commands::DELETEINCOMINGMESSAGE: if(deleteIncomingMessage(serv)) break; else return;
            case commands::SIGNOUT: if(signOut(serv)) break; else return;
       }
    }

}

bool signIn(const SOCKET& serv)
{
    // try to receive login and password length
    char lgSizeBuff[sizeof(int)];
    char pwSizeBuff[sizeof(int)];
    int lgSize;
    int pwSize;
    if(recvAll(serv, lgSizeBuff, sizeof(int), 0) <= 0) {
        cout<<"failed to receive login size!\n";
        return false;
    }
    if(recvAll(serv, pwSizeBuff, sizeof(int), 0) <= 0) {
        cout<<"failed to receive password size!\n";
        return false;
    }
    memcpy(&lgSize, lgSizeBuff, sizeof(int));
    memcpy(&pwSize, pwSizeBuff, sizeof(int));

    // now try to receive login and password

    char *lg = new char[lgSize]; // create buffers for login
    char *pw = new char[pwSize]; // and password

    if(recvAll(serv, lg, lgSize, 0) <= 0) {
        cout<<"failed to send login!";
        delete lg;                          // do not forget to free memory
        delete pw;                          // to avoid memory leaks
        return false;
    }

    if(recvAll(serv, pw, lgSize, 0) <= 0) {
        cout<<"failed to send password!";
        delete lg;
        delete pw;
        return false;
    }
    string login(lg), password(pw);
    delete lg;
    delete pw;
    // TODO: sql query to database!!!!!

    // here we are going to recv token
    // invalid token is "0000000000"
    char tok[10];

    memcpy(tok, "1111111111", 10);

    if(sendAll(serv, tok, 10, 0) == -1) {
        cout << "connection has been aborded!";
        return false;
    }
    return true;    // if all sucessfully ended
}

bool checkIncomingMessages(const SOCKET& serv)
{
    // first of all send token to have coversation with server
    bool onSuccess = getToken(serv);
    // if token correct - continue
    if(onSuccess)
    {
        //TODO: search messages in DB by token
        int numberOfIncMessages = 1;
        char numberOfIncMessagesBuff[sizeof(int)];
        memcpy(numberOfIncMessagesBuff, &numberOfIncMessages, sizeof(int));

        if(sendAll(serv, numberOfIncMessagesBuff, sizeof(int), 0) == -1) {
            cout << "failed to send number of msgs!";
            return false;
        }

        for(int i=0; i < numberOfIncMessages; i++) {
            //TODO sql query for message and send
            int sizeofMsg = sizeof("Hello world");
            char sizeofMsgBuff[sizeof(int)];
            memcpy(sizeofMsgBuff, &sizeofMsg, sizeof(int));
            if(sendAll(serv, sizeofMsgBuff, sizeof(int), 0) == -1) {
                cout << "failed to send message size!";
                return false;
            }

            if(sendAll(serv, "hello world", 11, 0) == -1) {
                cout << "failed to send message!";
                return false;
            }
        }

        return true;
    } else
        return false;
}

bool deleteIncomingMessage(const SOCKET& serv)
{
    // try to receive "ticket"
    bool onSuccess = getToken(serv);
    // if token correct - continue
    if(onSuccess)
    {
        // here receiving number of message to delete
        char buff[sizeof(int)];
        int numOfmsg;

        if(recvAll(serv, buff, sizeof(int), 0) <= 0) {
            cout<<"error with sending number of deleting message!";
            return false;
        }
        memcpy(&numOfmsg, buff, sizeof(int));
        // TODO delete message from DB by number


        return true;
    } else
        return false;
}

bool signOut(const SOCKET& serv)
{
    return getToken(serv);
}

bool getToken(const SOCKET& serv)
{
    // receive token
    char tok[10];
    if(recvAll(serv, tok, 10, 0) <= 0) {
        cout<<"error with sending token!";
        return false;
    }

    char answer = 1;


    //TODO: check token in DB and form an answer and send him to the client
    if(sendAll(serv, &answer, sizeof(char), 0) == -1) {
        cout << "error to send answer about token!";
        return false;
    }
    // if token not rigth - disconnect client
    if(answer == '0') {
        return false;
    }
    return true;
}

int sendAll(const SOCKET& serv, const char *buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = send(serv, buf + total, len-total, flags);
        if(n == SOCKET_ERROR) { break; }
        total += n;
    }

    return (n == SOCKET_ERROR ? -1 : total);
}

int recvAll(const SOCKET& serv, char* buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = recv(serv, buf + total, len - total, flags);
        if(n == 0 || n == SOCKET_ERROR) { break; } // client disconnected
        total += n;
    }

    if(n == 0) return 0;                    // if the connection has been gracefully closed
    else
        if(n == SOCKET_ERROR) return -1;    // if the connection has been aborded
            else
                return n;
}












