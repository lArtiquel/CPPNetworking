#include "dbfunctions.h"

DataBase::DataBase()
{

}

bool DataBase::connectToDataBase()
{
    if(!QFile("T:/Artyom/DataBases/" DATABASE_NAME).exists()){
        restoreDataBase();
    } else {
        openDataBase();
    }
}

string DataBase::getToken(const string& login, const string& password)
{

}

int DataBase::getNumberOfMessages()
{
    QSqlQuery query;
    query.prepare("SELECT token FROM auth WHERE login="+login+" AND password="+password+';')
    //TODO how to get token
}

bool DataBase::inserIntoDeviceTable(const QVariantList &data)
{
    /* Запрос SQL формируется из QVariantList,
     * в который передаются данные для вставки в таблицу.
     * */
    QSqlQuery query;
    /* В начале SQL запрос формируется с ключами,
     * которые потом связываются методом bindValue
     * для подстановки данных из QVariantList
     * */
    query.prepare("INSERT INTO " DEVICE " ( " DEVICE_HOSTNAME ", "
                                              DEVICE_IP ", "
                                              DEVICE_MAC " ) "
                  "VALUES (:Hostname, :IP, :MAC )");
    query.bindValue(":Hostname",    data[0].toString());
    query.bindValue(":IP",          data[1].toString());
    query.bindValue(":MAC",         data[2].toString());
    // После чего выполняется запросом методом exec()
    if(!query.exec()){
        qDebug() << "error insert into " << DEVICE;
        qDebug() << query.lastError().text();
        return false;
    } else {
        return true;
    }
    return false;
}

bool DataBase::openDataBase()
{
    /* База данных открывается по заданному пути
     * и имени базы данных, если она существует
     * */
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("T:/Artyom/DataBases/" DATABASE_NAME);
    if(db.open()){
        return true;
    } else {
        return false;
    }
}

bool DataBase::restoreDataBase()
{
    if(this->openDataBase()){
        if(!this->createDeviceTable()){
            return false;
        } else {
            return true;
        }
    } else {
        qDebug() << "failed to restore database";
        return false;
    }
}

void DataBase::closeDataBase()
{
    db.close();
}

bool DataBase::createDeviceTable()
{
    /* В данном случае используется формирование сырого SQL-запроса
     * с последующим его выполнением.
     * */
    QSqlQuery query;
    if(!query.exec( "CREATE TABLE `auth`"
                    " ( `login` TEXT NOT NULL UNIQUE, `password` TEXT NOT NULL, `token` TEXT NOT NULL UNIQUE )")
                    ||
        !query.exec("CREATE TABLE `mailDB`"
                    " ( `token` TEXT NOT NULL, `incomingmsg` TEXT NOT NULL )")){
        qDebug() << "error of create database";
        qDebug() << query.lastError().text();
        return false;
    } else {
        qDebug()<< "database restored!\n";
        return true;
    }
}

DataBase::~DataBase()
{
    closeDataBase();
}


