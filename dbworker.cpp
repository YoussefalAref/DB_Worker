#include "dbworker.h"

DBWorker::DBWorker() :
    working_db(QSqlDatabase::addDatabase("QSQLITE")),
    qry(new QSqlQuery(working_db))
{
}

QStringList DBWorker::accessing_db(QString db_path)
{
    if (working_db.isOpen()) {
        working_db.close();
    }

    working_db.setDatabaseName(db_path);
    if (!working_db.open()) {
        qDebug() << "Database error:" << working_db.lastError().text();
        return QStringList();
    }

    return working_db.tables();
}

QSqlQuery* DBWorker::loading_table(QString chosen_table)
{
    qry->finish(); // Clear previous query
    if (!qry->exec("SELECT * FROM " + chosen_table)) {
        qDebug() << "Query error:" << qry->lastError().text();
    }
    return qry;
}




