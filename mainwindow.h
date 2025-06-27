#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "dbworker.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QPushButton>
#include <QWidget>
#include <QGridLayout>
#include <QComboBox>
#include <QTableWidget>
#include <QDir>
#include <QDebug>
#include <QString>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QMessageBox>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString file_path;
    QString current_mode;
    QStringList tables_list;
    DBWorker *db_manager;
    QComboBox* db_tables;
    QTableWidget* db_selected_table;
    bool save_changes=true;

private slots:
    void button_toggled(bool value);
    void load_button_clicked();
    void add_toggled(bool);
    void edit_toggled(bool);
    void remove_toggled(bool);
    void delete_selected_rows();
    void update_button_clicked();

private:
    void check_saving_changes();

};

#endif // MAINWINDOW_H
