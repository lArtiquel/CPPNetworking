#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h> //need it for fork()
#include <sys/types.h>//also
#include <sys/wait.h>//for waitpid
#include <signal.h>
#include <netdb.h>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QDebug>

using namespace std;

struct sessionRes
{
    int mark;
    QString name;
    QString surname;
};

QVector<sessionRes> vec;

void readDataFromFile()
{
    QFile file("info");
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"file not opened!\n";
        return;
    }

    QTextStream in(&file);
    vec.clear();
    while(true)
    {
        sessionRes temp;
        temp.name = in.readLine();
        if(temp.name == "") break;
        temp.surname = in.readLine();
        if(temp.name == "") break;
        in>>temp.mark; in.readLine();
        vec.push_back(temp);
    }
    file.close();
}

void writeDataToFile()
{
    QFile file("info");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "file not opened!";
        return;
    }

    QTextStream out(&file);
    for(int i = 0; i < vec.size(); i++){
        out<<vec.at(i).name<<'\n'<<vec.at(i).surname<<'\n'<<vec.at(i).mark;
        if(i != vec.size() - 1) {
            out << '\n';
        }
    }
    file.close();
}

int sendall(int s, const char *buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = send(s, buf + total, len - total, flags);
        if(n == -1) { break; }
        total += n;
    }

    return (n==-1 ? -1 : total);
}

int recvall(int s, char* buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = recv(s, buf + total, len - total, flags);
        if(n == 0) {break;}                             //client disconnected
        total += n;
    }

    return (n==0 ? 0 : total);
}

void reaper(int sig)        //execute signal of reaping child
{
    while (waitpid(-1, NULL, WNOHANG) > 0) //-1 for all childs
    {
        qDebug() << "child terminated!";
    }
}

int procClient(int client)  //client process function
{
    while(true)
    {
        int command;
        char commandBuf[sizeof(int)];
        if(recvall(client, commandBuf, sizeof(int), 0) == 0) return 8;
        memcpy(&command, commandBuf, sizeof(int));

        switch(command)
        {
            case 1: //view
            {
                readDataFromFile();
                QString str("All students:\n");
                for(int i = 0; i<vec.size(); i++){
                    str.append(QString::number(i+1) + ' ' + vec.at(i).name+' '+vec.at(i).surname+' '+QString::number(vec.at(i).mark)+'\n');
                }

                char messageSizeBuf[sizeof(int)];
                int size = str.size();

                memcpy(messageSizeBuf, &size, sizeof(int));
                if(sendall(client, messageSizeBuf, sizeof(int), 0) == -1) return 7;
                if(sendall(client, str.toStdString().c_str(), size, 0) == -1) return 7;
                break;
            }

            case 2: //add
            {
                sessionRes temp;
                int numberOfIncomingBytes = 0;
                char numberOfIncomingBytesBuf[sizeof(int)];

                //receive name length
                if(recvall(client, numberOfIncomingBytesBuf, sizeof(int), 0) == 0) return 8;
                //receive name
                memcpy(&numberOfIncomingBytes, numberOfIncomingBytesBuf, sizeof(int));
                char name[numberOfIncomingBytes + 1];
                if(recvall(client, name, numberOfIncomingBytes, 0) == 0) return 8;
                name[numberOfIncomingBytes] = '\0';
                temp.name = name;

                //receive surname length
                if(recvall(client, numberOfIncomingBytesBuf, sizeof(int), 0) == 0) return 8;
                //receive surname
                memcpy(&numberOfIncomingBytes, numberOfIncomingBytesBuf, sizeof(int));
                char surname[numberOfIncomingBytes + 1];
                if(recvall(client, surname, numberOfIncomingBytes, 0) == 0) return 8;
                surname[numberOfIncomingBytes] = '\0';
                temp.surname = surname;

                //receive mark
                if(recvall(client, numberOfIncomingBytesBuf, sizeof(int), 0) == 0) return 8;
                int mark;
                memcpy(&mark, numberOfIncomingBytesBuf, sizeof(int));
                temp.mark = mark;

                readDataFromFile();
                vec.push_back(temp);
                writeDataToFile();
                break;
            }

            case 3: //delete
            {
                readDataFromFile();

                int numberOfStud = 0;
                char numberOfStudBuf[sizeof(int)];
                if(recvall(client, numberOfStudBuf, sizeof(int), 0) == 0) return 9;

                memcpy(&numberOfStud, numberOfStudBuf, sizeof(int));

                if((numberOfStud >= 1) && (numberOfStud <= vec.size())){
                    vec.erase(vec.begin()+numberOfStud-1);
                    writeDataToFile();
                }
                break;
            }

            default:return 0;
        }
    }
}

int main()
{
    int serverSocket, clientSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        perror("creation socket failed!\n");
        exit(1);
    }

    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    sockaddr_in addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
    addrServer.sin_port = htons(50001);

    //bind socket with structure
    if(bind(serverSocket, (sockaddr *) &addrServer, sizeof(addrServer))<0){
        perror("binding error");
        exit(2);
    }

    // Listen for incoming connection requests.
    listen(serverSocket, SOMAXCONN);
    //listen for signals from other proc
    signal(SIGCHLD, reaper);

    while (1)
    {
        if((clientSocket = accept(serverSocket, nullptr, nullptr)) < 0){
            qDebug() << "acception error\n";
            exit(3);
        }

        switch(fork())
        {
            case -1:
                    qDebug() << "forking error";
                    break;
            case 0:
                    {
                        close(serverSocket);       //if this is child process
                        int retval = procClient(clientSocket);
                        close(clientSocket);
                        return retval;
                    }
            default:
                    close(clientSocket);       //if this parent process
        }
    }
    return 0;
}
