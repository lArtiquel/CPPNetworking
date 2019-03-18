#ifndef DBFUNCTIONS_H
#define DBFUNCTIONS_H
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QFile>
#include <string>

using namespace std;

#define DATABASE_NAME

class DataBase{
public:
    DataBase();
    ~DataBase();
    bool getResult() const;

    bool connectToDataBase();
    string getToken(const string&, const string&);
    int getNumberOfMessages();
    string getMessageByNumber(int);
    void deleteMessgaeByNumber(int);

private:
    bool result;
    QSqlDatabase db;

    bool openDataBase();
    bool restoreDataBase();
    void closeDataBase();
    bool createDeviceTable();
};





#endif // DBFUNCTIONS_H
