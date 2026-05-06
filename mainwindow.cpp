#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "source/AddTransaction.h"
#include "source/AddCategory.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QDir>
#include <QMessageBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1200, 800);

    QDir dir;
    if (!dir.exists("databases")) dir.mkpath("databases");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/macie/Documents/FinanceManager/databases/finances.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Błąd", "Nie udało się połączyć: " + db.lastError().text());
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS categories (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, type TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS transactions (id INTEGER PRIMARY KEY AUTOINCREMENT, amount REAL NOT NULL, date TEXT NOT NULL, description TEXT, category_id INTEGER, created_at TEXT)");

    model = new QSqlQueryModel(this);
    refreshTable();

    ui->tableView->setModel(model);
    ui->tableView->hideColumn(0);
    model->setHeaderData(1, Qt::Horizontal, "Kwota");
    model->setHeaderData(2, Qt::Horizontal, "Typ");
    model->setHeaderData(3, Qt::Horizontal, "Kategoria");
    model->setHeaderData(4, Qt::Horizontal, "Data");
    model->setHeaderData(5, Qt::Horizontal, "Opis");
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    AddTransaction *addTransWidget = new AddTransaction(this);
    ui->stackedWidget->addWidget(addTransWidget);

    connect(addTransWidget, &AddTransaction::transactionAdded, this, &MainWindow::refreshTable);

    AddCategory *addCatWidget = new AddCategory(this);
    ui->stackedWidget->addWidget(addCatWidget);

    connect(addCatWidget, &AddCategory::categoryAdded, addTransWidget, &AddTransaction::loadCategories);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshTable()
{
    model->setQuery("SELECT t.id, t.amount, c.type, c.name, t.date, t.description "
                    "FROM transactions t "
                    "LEFT JOIN categories c ON t.category_id = c.id");
}

void MainWindow::on_pushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}