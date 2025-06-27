#ifndef DBWORKER_H
#define DBWORKER_H

#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>

class DBWorker
{
public:
    DBWorker();
    QStringList accessing_db(QString);
    QSqlQuery* loading_table(QString);
    void add_row();
    void remove_rows();
    void edit_data();

private:
    QSqlDatabase working_db;
    QSqlQuery *qry;

};

#endif // DBWORKER_H
