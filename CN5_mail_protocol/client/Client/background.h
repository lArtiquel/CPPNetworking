#ifndef BACKGROUND_H
#define BACKGROUND_H
#include <WinSock2.h>
#include <QObject>
#include <QString>

class Background : public QObject
{
    Q_OBJECT
public:
    explicit Background(QObject *parent = nullptr);
    Q_INVOKABLE bool connectToServer(const QString&, const QString&);
    Q_INVOKABLE int checkIncMessages();
    Q_INVOKABLE bool deleteIncMessage(const int&);
    Q_INVOKABLE bool signOut();
    Q_INVOKABLE QString getMessage();

private:
    bool signIn(const QString&, const QString&);
    bool sendToken();
    bool sendCommandToServer(const int&);
    int sendAll(const char *buf, int len, int flags);
    int recvAll(char *buf, int len, int flags);

    enum {SIGNIN, CHECKINCOMINGMESSAGES, DELETEINCOMINGMESSAGE, SIGNOUT};
    QString token;
    WSAData wsadata;
    SOCKET sock;
    sockaddr_in addrserver;
};

#endif // BACKGROUND_H
