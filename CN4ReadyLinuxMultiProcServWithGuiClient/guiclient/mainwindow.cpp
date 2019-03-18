#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <unistd.h> //need it for closing socket


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->viewButton, SIGNAL(clicked()), this, SLOT(viewStudents()));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addStudents()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteStudents()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectToServer()));
    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectFromServer()));
    //by default this butons locked until we connect to server
    locker(true);
}

MainWindow::~MainWindow()
{
    disconnectFromServer(); //for closing socket
    delete ui;
}

void MainWindow::disconnectFromServer()
{
    char sendingBuf[sizeof(int)];
    int command = DISCONNECT;
    memcpy(sendingBuf, &command, sizeof(int));
    if(sendAll(sock, sendingBuf, sizeof(int), 0) == -1) {  //send to server what we are disconnecting
        ui->textEdit->append("error with right disconnection\n");
    }

    locker(true);                                  //unlock connect button
    ::close(sock);                                 //free memory socket
}

void MainWindow::viewStudents()
{
    char sendingBuf[sizeof(int)];
    int command = VIEW;
    memcpy(sendingBuf, &command, sizeof(int));
    if(sendAll(sock, sendingBuf, sizeof(int), 0) == -1) {   //send view command to server
        ui->textEdit->append("error with sending request\n");
        disconnectFromServer();
        return;
    }

    //receive response size
    int responseSize;
    char responseSizeBuf[sizeof(int)];
    if(recvAll(sock, responseSizeBuf, sizeof(int), 0) == 0){
        ui->textEdit->append("error with receiving size\n");
        disconnectFromServer();
        return;
    }
    memcpy(&responseSize, responseSizeBuf, sizeof(int));

    //receive response
    char responseBuf[responseSize+1];
    if(recvAll(sock, responseBuf, responseSize, 0) == 0){
        ui->textEdit->append("error with receiving response\n");
        disconnectFromServer();
        return;
    }
    responseBuf[responseSize] = '\0';       //add null-term
    QString response(responseBuf);
    ui->textEdit->append(response);
}

void MainWindow::addStudents()
{
    addingDialog addDialog(this);
    int dialogCode = addDialog.exec();

    QString name;
    QString surname;
    int mark;

    if(dialogCode == QDialog::Rejected)
    {
        return;
    } else
    {
        name = addDialog.getName();
        surname = addDialog.getSurname();
        mark = addDialog.getMark();
    }
    //send ADD command first of all
    char sendingBuf[sizeof(int)];
    int command = ADD;
    memcpy(sendingBuf, &command, sizeof(int));
    if(sendAll(sock, sendingBuf, sizeof(int), 0) == -1) {   //send view command to server
        ui->textEdit->append("error with sending request\n");
        disconnectFromServer();
        return;
    }     else {
        //send the length of the name
        int size = name.size();
        memcpy(sendingBuf, &size, sizeof(int));
        if(sendAll(sock, sendingBuf, sizeof(int), 0) == -1) {
            ui->textEdit->append("error with sending name length\n");
            disconnectFromServer();
            return;
        }
        //then send the name
        if(sendAll(sock, name.toStdString().c_str(), size, 0) == -1) {    //send with \0
            ui->textEdit->append("error with sending name\n");
            disconnectFromServer();
            return;
        }
        //send the length of surname
        size = surname.size();
        memcpy(sendingBuf, &size, sizeof(int));
        if(sendAll(sock, sendingBuf, sizeof(int), 0) == -1) {
            ui->textEdit->append("error with sending surname length\n");
            disconnectFromServer();
            return;
        }
        //send surname
        if(sendAll(sock, surname.toStdString().c_str(), size, 0) == -1) {    //send with \0
            ui->textEdit->append("error with sending surname\n");
            disconnectFromServer();
            return;
        }

        char markBuffer[sizeof(int)];
        memcpy(markBuffer, &mark, sizeof(int));
        if(sendAll(sock, markBuffer, sizeof(int), 0) == -1) {    //send with \0
            ui->textEdit->append("error with sending mark\n");
            disconnectFromServer();
            return;
        } else {
            ui->textEdit->append("Student successfully added!\n");
        }
    }
}

void MainWindow::deleteStudents()
{
    DeletingDialog deleteDialog(this);
    int dialogCode = deleteDialog.exec();
    int numberOfStudent;

    if(dialogCode == QDialog::Rejected) {
        return;
    } else {
        numberOfStudent = deleteDialog.getNumberOfStudent();
    }

    char sendingBuf[sizeof(int)];
    int command = DELETE;
    memcpy(sendingBuf, &command, sizeof(int));
    if(sendAll(sock, sendingBuf, sizeof(int), 0) == -1) {   //send delete command to server
        ui->textEdit->append("error with sending request\n");
        disconnectFromServer();
        return;
    }     else {
        char numOfStudBuf[sizeof(int)];
        memcpy(numOfStudBuf, &numberOfStudent, sizeof(int));
        if(sendAll(sock, numOfStudBuf, sizeof(int), 0) == -1) {    //send with \0
            ui->textEdit->append("error with sending number of student\n");
            disconnectFromServer();
            return;
        } else {
            ui->textEdit->append("user successfully deleted if you choose right!\n");
        }
    }
}

void MainWindow::connectToServer()
{
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        ui->textEdit->append("error creation socket\n");
        return;
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(50001);
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(::connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
        ui->textEdit->append("error with connection to server " +
                              QString::number(servaddr.sin_addr.s_addr) +
                              " port " + QString::number(servaddr.sin_port) + '\n');
        return;
    }
    locker(false);  //unlock buttons
}

void MainWindow::locker(bool isTrue)
{
    if(isTrue){
        ui->viewButton->setDisabled(true);
        ui->addButton->setDisabled(true);
        ui->deleteButton->setDisabled(true);
        ui->connectButton->setDisabled(false);
        ui->disconnectButton->setDisabled(true);
    } else {
        ui->viewButton->setDisabled(false);
        ui->addButton->setDisabled(false);
        ui->deleteButton->setDisabled(false);
        ui->connectButton->setDisabled(true);
        ui->disconnectButton->setDisabled(false);
    }
}

int MainWindow::sendAll(int s, const char *buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = send(s, buf + total, len-total, flags);
        if(n == -1) { break; }
        total += n;
    }

    return (n==-1 ? -1 : total);
}

int MainWindow::recvAll(int s, char* buf, int len, int flags)
{
    int total = 0;
    int n;

    while(total < len)
    {
        n = recv(s, buf + total, len - total, flags);
        if(n == 0) {break;} //client disconnected
        total += n;
    }

    return (n==0 ? 0 : total);
}
