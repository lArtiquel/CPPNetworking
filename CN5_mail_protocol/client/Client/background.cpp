#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "background.h"
#include <QDebug>
#include <QCoreApplication>

Background::Background(QObject *parent) : QObject(parent)
{
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (iResult != NO_ERROR) {
        qDebug()<< "WSAStartup failed with error: " + QString::number(iResult);
        QCoreApplication::exit(1);
    }
}

bool Background::connectToServer(const QString& login, const QString& pass)
{
    //----------------------
    // Create a SOCKET for connecting to server
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        qDebug()<<"error with creating socket!";
        WSACleanup();
        QCoreApplication::exit(1);
    }
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    sockaddr_in addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServer.sin_port = htons(20131);

    //----------------------
    // Connect to server.
    int iResult = ::connect(sock, (SOCKADDR*)&addrServer, sizeof(addrServer));
    if (iResult == SOCKET_ERROR) {
        closesocket(sock);
        qDebug()<<"error with connecting!";
        return false;
    }

    // try signIn system
    bool onSuccess = signIn(login, pass);
    return onSuccess;
}

bool Background::signIn(const QString& login, const QString& pass)
{
    // send to server command to singIn
    if(sendCommandToServer(SIGNIN) == false) return false;

    // send length of password
    int lgSize = login.size();
    int pwSize = pass.size();
    char lgSizeBuff[sizeof(int)];
    char pwSizeBuff[sizeof(int)];
    memcpy(lgSizeBuff, &lgSize, sizeof(int));
    memcpy(pwSizeBuff, &pwSize, sizeof(int));
    if(sendAll(lgSizeBuff, sizeof(int), 0) == -1) {
        qDebug()<<"failed to send login size!\n";
        return false;
    }
    if(sendAll(pwSizeBuff, sizeof(int), 0) == -1) {
        qDebug()<<"failed to send password size!\n";
        return false;
    }

    // now try to send login and password
    char *lg = new char[lgSize]; // create buffers for login
    char *pw = new char[pwSize]; // and password
    memcpy(lg, login.toStdString().c_str(), login.size());
    memcpy(pw, pass.toStdString().c_str(), pass.size());

    if(sendAll(lg, lgSize, 0) == -1) {
        qDebug()<<"failed to send login!";
        delete lg;                          // do not forget to free memory
        delete pw;                          // to avoid memory leaks
        return false;
    }

    if(sendAll(pw, pwSize, 0) == -1) {
        qDebug()<<"failed to send password!";
        delete lg;
        delete pw;
        return false;
    }

    delete lg;
    delete pw;

    // here we are going to recv token
    // invalid token is "0000000000"
    char tok[10];

    if(recvAll(tok, 10, 0) <= 0) {
        qDebug() << "connection has been aborded!";
        return false;
    } else {
        token.clear();  // delete prev token
        token = QString::fromLatin1(tok, 10);
        if(token == "0000000000")
        {
           return false;        // if password & login are wrong
        } else
        {
           return true;
        }
    }
}

int Background::checkIncMessages()
{
    // try to send command to server
    if(!sendCommandToServer(CHECKINCOMINGMESSAGES)) return -1;
    // first of all send token to have coversation with server
    bool onSuccess = sendToken();
    // if token correct - continue
    if(onSuccess)
    {
        // now if token is rigth - continue to get messages
        char buff[sizeof(int)];
        int retval = recvAll(buff, sizeof(int), 0);
        if(retval == 0) {
            qDebug() << "connection has been gracefully closed!";
            return -1;
        } else
            if(retval == SOCKET_ERROR) {
                qDebug() << "connection has been aborded!";
                return -1;
            } else {
                memcpy(&retval, buff, sizeof(int));
                return retval;          // return number of incoming messages
            }
    } else
        return -1;
}

QString Background::getMessage()
{
    int msgSize;
    char msgSizeBuff[sizeof(int)];
    // receive length of message
    if(recvAll(msgSizeBuff, sizeof(int), 0) <= 0) {
        qDebug() << "connection has been closed!";
        QString empty("");
        return empty;
    }
    // convert buffer to int
    memcpy(&msgSize, msgSizeBuff, sizeof(int));
    // receive message
    char *msgBuf = new char[msgSize];
    if(recvAll(msgBuf, msgSize, 0) <= 0) {
        qDebug() << "connection has been closed!";
        delete msgBuf;
        QString empty("");
        return empty;
    }
    // if we are successfully received the raw massage
    delete msgBuf;
    QString msg(msgBuf);
    return msg;
}

bool Background::deleteIncMessage(const int& numOfmsg)
{
    if(!sendCommandToServer(DELETEINCOMINGMESSAGE)) return false;

    // try to send "ticket"
    bool onSuccess = sendToken();
    // if token correct - continue
    if(onSuccess)
    {
        //here send to server number of msg to delete
        char buff[sizeof(int)];
        memcpy(buff, &numOfmsg, sizeof(int));
        if(sendAll(buff, sizeof(int), 0) == -1) {
            qDebug()<<"error with sending number of deleting message!";
            return false;
        }

        return true;
    } else
        return false;
}

bool Background::signOut()
{
    if(!sendCommandToServer(SIGNOUT)) return false;
    if(!sendToken()) return false;
    return true;
}

bool Background::sendToken()
{
    char *tok = token.toLatin1().data();
    if(sendAll(tok, 10, 0) == -1) {
        qDebug()<<"error with sending token!";
        return false;
    }
    // accept answer about token
    char answer;
    if(recvAll(&answer, sizeof(char), 0) <= 0) {
        qDebug() << "errror when we tried to accept answer about token!";
        return false;
    }
    // here we are trying to read answer('0'-irregular token)
    if(answer == '0')
    {
        qDebug() << "error: irregular token!";
        return false;
    }
    return true;
}

bool Background::sendCommandToServer(const int & cmd)
{
    char buff[sizeof(int)];
    // serialize number to char array
    memcpy(buff, &cmd, sizeof(int));
    if(sendAll(buff, sizeof(int), 0) == -1) {
        qDebug()<<"error with sending command: "<< cmd<< '\n';
        return false;
    }
    return true;
}

int Background::sendAll(const char *buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = send(sock, buf + total, len-total, flags);
        if(n == SOCKET_ERROR) { break; }
        total += n;
    }

    return (n == SOCKET_ERROR ? -1 : total);
}

int Background::recvAll(char* buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = recv(sock, buf + total, len - total, flags);
        if(n == 0 || n == SOCKET_ERROR) { break; } // client disconnected
        total += n;
    }

    if(n == 0) return 0;                    // if the connection has been gracefully closed
    else
        if(n == SOCKET_ERROR) return -1;    // if the connection has been aborded
            else
                return n;
}








