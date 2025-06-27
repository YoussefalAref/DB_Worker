#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    db_manager(new DBWorker),
    db_tables(new QComboBox),
    db_selected_table(new QTableWidget)
{
    ui->setupUi(this);

    // Setup central widget and layout
    QWidget* center = new QWidget(this);
    setCentralWidget(center);
    QGridLayout* main_layout = new QGridLayout(center);

    // Database controls
    QPushButton* open = new QPushButton("Open");
    QPushButton* load = new QPushButton("Load");
    main_layout->addWidget(open, 0, 0);
    main_layout->addWidget(db_tables, 0, 1, 1, 2);
    main_layout->addWidget(load, 0, 3);

    // Table display
    main_layout->addWidget(db_selected_table, 1, 0, 1, 4);
    db_selected_table->setEditTriggers(QAbstractItemView::NoEditTriggers); // Default: No editing

    // Modification buttons
    QPushButton* add = new QPushButton("Add");
    add->setCheckable(true);
    QPushButton* remove = new QPushButton("Remove");
    remove->setCheckable(true);
    QPushButton* edit = new QPushButton("Edit");
    edit->setCheckable(true);
    main_layout->addWidget(add, 2, 0);
    main_layout->addWidget(remove, 2, 1);
    main_layout->addWidget(edit, 2, 2);

    // Update button
    QPushButton* update = new QPushButton("Update");
    main_layout->addWidget(update, 2, 3);

    // Connections
    connect(open, &QPushButton::clicked, this, [this] {
        file_path = QFileDialog::getOpenFileName(this, "Open Database", QDir::homePath(), "SQLite Databases (*.db *.sqlite)");
        if (!file_path.isEmpty()) {
            tables_list = db_manager->accessing_db(file_path);
            db_tables->clear();
            db_tables->addItems(tables_list);
            db_selected_table->setColumnCount(0);
            db_selected_table->setRowCount(0);
        }
    });

    connect(load, &QPushButton::clicked, this, &MainWindow::load_button_clicked);
    connect(add, &QPushButton::toggled, this, &MainWindow::add_toggled);
    connect(remove, &QPushButton::toggled, this, &MainWindow::remove_toggled);
    connect(edit, &QPushButton::toggled, this, &MainWindow::edit_toggled);
    connect(update, &QPushButton::clicked, this, &MainWindow::update_button_clicked);
}

MainWindow::~MainWindow()
{
    delete db_manager;
    delete ui;
}

void MainWindow::load_button_clicked()
{
    QString sel_table = db_tables->currentText();
    if (sel_table.isEmpty()) return;

    QSqlQuery* current_qry = db_manager->loading_table(sel_table);
    if (!current_qry->isActive()) {
        qDebug() << "Query failed:" << current_qry->lastError().text();
        return;
    }

    QSqlRecord current_record = current_qry->record();
    int column_count = current_record.count();

    // Setup table headers
    db_selected_table->setColumnCount(column_count);
    db_selected_table->setRowCount(0);

    QStringList headers;
    for (int i = 0; i < column_count; ++i) {
        headers << current_record.fieldName(i);
    }
    db_selected_table->setHorizontalHeaderLabels(headers);


    int row_count = 0;
    while (current_qry->next()) {
        db_selected_table->insertRow(row_count);
        for (int i = 0; i < column_count; ++i) {
            QTableWidgetItem* tableItm = new QTableWidgetItem(current_qry->value(i).toString());
            tableItm->setFlags(tableItm->flags());
            db_selected_table->setItem(row_count, i, tableItm);
        }
        row_count++;
    }
}

void MainWindow::button_toggled(bool value)
{
    if (db_selected_table->rowCount() != 0) {
        QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
        if (!senderButton) return;
        check_saving_changes();

        if (value) {
            current_mode = senderButton->text();
            senderButton->setStyleSheet("background-color: green");
            foreach(QPushButton *button, findChildren<QPushButton*>()) {
                if (button != senderButton && button->isCheckable()) {
                    button->setChecked(false);
                }
            }
        } else {
            senderButton->setStyleSheet("background-color: white");
            current_mode = "";
        }
    }
    else {
        QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
        senderButton->setChecked(false);
    }
}

void MainWindow::add_toggled(bool value)
{
    bool added_row = false;
    button_toggled(value);
    if(db_selected_table->rowCount()!=0){
    current_mode=value? "add":"";

    if (value && db_selected_table->rowCount() != 0)
    {
        int newRow = db_selected_table->rowCount();
        db_selected_table->insertRow(newRow);
        added_row = true;

        // Make new row editable
        for (int col = 0; col < db_selected_table->columnCount(); ++col)
        {
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setFlags(value ? (item->flags() | Qt::ItemIsEditable)
                                : (item->flags() & ~Qt::ItemIsEditable));
            db_selected_table->setItem(newRow, col, item);
        }

        db_selected_table->setCurrentCell(newRow, 0);
    }

    // Enable/disable edit triggers
    db_selected_table->setEditTriggers(value ? QAbstractItemView::AllEditTriggers
                                            : QAbstractItemView::NoEditTriggers);

    if (added_row)
        save_changes = false;
}
}

void MainWindow::remove_toggled(bool value)
{
    button_toggled(value);
    if(db_selected_table->rowCount()!=0){
    current_mode=value? "remove":"";
    if (value) {
        db_selected_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        connect(db_selected_table->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MainWindow::delete_selected_rows);
    } else {
        db_selected_table->setSelectionBehavior(QAbstractItemView::SelectItems);
        disconnect(db_selected_table->selectionModel(), &QItemSelectionModel::selectionChanged,
                   this, &MainWindow::delete_selected_rows);
    }
    }
}

void MainWindow::edit_toggled(bool value)
{
    button_toggled(value);
    int rows_count=db_selected_table->rowCount();
    if(rows_count!=0){
    current_mode=value? "edit":"";
    if(value){

        db_selected_table->setEditTriggers(QAbstractItemView::AllEditTriggers);
        connect(db_selected_table,&QTableWidget::itemChanged,this,[this](QTableWidgetItem* itm){save_changes=false;});
    }
    else {
    db_selected_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }


}}

void MainWindow::delete_selected_rows()
{
    if (current_mode != "remove") return;

    QList<QTableWidgetSelectionRange> ranges = db_selected_table->selectedRanges();
    if (ranges.isEmpty()) return;

    int reply = QMessageBox::question(this, "Delete Rows",
                                    "Delete selected rows?",
                                    QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QList<int> rowsToDelete;
    for (const QTableWidgetSelectionRange &range : ranges) {
        for (int row = range.topRow(); row <= range.bottomRow(); ++row) {
            rowsToDelete << row;
        }
    }

    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());

    foreach (int row, rowsToDelete) {
        db_selected_table->removeRow(row);
    }
    save_changes = false;
}

void MainWindow::check_saving_changes()
{
    if (!save_changes)
    {
        int reply = QMessageBox::question(this, "Unsaved Changes",
                                        "Do you want to save your edits?",
                                        QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
            update_button_clicked();
        else
            load_button_clicked();
            save_changes=true;
    }
}

void MainWindow::update_button_clicked()
{
    qDebug()<<current_mode;
    if(current_mode=="add")
    {
       db_manager->add_row();

    }
    else if (current_mode=="remove")
    {
        db_manager->remove_rows();
    }
    else
    {
        db_manager->edit_data();
    }
    save_changes = true;
    current_mode="";
}
