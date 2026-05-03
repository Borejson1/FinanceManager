#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/macie/Documents/FinanceManager/databases/finances.db");

    if (!db.open()) {
        qDebug() << "Błąd bazy:" << db.lastError().text();
        return;
    } else {
        qDebug() << "Baza działa";
    }

    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("transactions");
    model->select();

    ui->tableView->setModel(model);
}

MainWindow::~MainWindow()
{
    delete ui;
}