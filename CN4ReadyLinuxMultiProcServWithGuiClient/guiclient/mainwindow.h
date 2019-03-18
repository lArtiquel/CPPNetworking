#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "addingdialog.h"
#include "deletingdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void viewStudents();
    void addStudents();
    void deleteStudents();
    void connectToServer();
    void disconnectFromServer();

private:
    Ui::MainWindow *ui;
    enum {DISCONNECT, VIEW, ADD, DELETE};
    void locker(bool);
    int sendAll(int, const char *, int, int);
    int recvAll(int, char *, int, int);
    int sock;
    struct sockaddr_in servaddr;
};

#endif // MAINWINDOW_H
