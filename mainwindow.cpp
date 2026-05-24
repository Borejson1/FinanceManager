#include "mainwindow.h"
#include "financechartmanager.h"
#include "ui_mainwindow.h"
#include "source/AddTransaction.h"
#include "source/AddCategory.h"

#include <QStandardPaths>
#include <QDir>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QDir>
#include <QMessageBox>
#include <QHeaderView>

#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QPieSeries>
#include <QPieSlice>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1200, 800);

    currentChartMode = 1;

    QDir dir;
    if (!dir.exists("databases")) dir.mkpath("databases");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    db.setDatabaseName(path + "/finances.db");

    if (!db.open()) {

        QMessageBox::critical(this, "Błąd", "Nie udało się połączyć: " + db.lastError().text());
        return;
    }
    transactionModel = new QSqlTableModel(this);
    transactionModel->setTable("transactions");
    transactionModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    transactionModel->select();

    ui->tableView->setModel(transactionModel);
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS categories (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, type TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS transactions (id INTEGER PRIMARY KEY AUTOINCREMENT, amount REAL NOT NULL, date TEXT NOT NULL, description TEXT, category_id INTEGER, created_at TEXT)");

    dashboardWidget = new QWidget(this);
    QHBoxLayout *dashboardLayout = new QHBoxLayout(dashboardWidget);

    QVBoxLayout *chartMenuLayout = new QVBoxLayout();
    QPushButton *btnChart1 = new QPushButton("Całkowity Bilans", this);
    QPushButton *btnChart2 = new QPushButton("Wydatki wg Kategorii", this);
    QPushButton *btnChart3 = new QPushButton("Przychody wg Kategorii", this);

    chartMenuLayout->addWidget(btnChart1);
    chartMenuLayout->addWidget(btnChart2);
    chartMenuLayout->addWidget(btnChart3);
    chartMenuLayout->addStretch();

    connect(btnChart1, &QPushButton::clicked, this, [this]() {
        currentChartMode = 1;
        updateChart();
    });

    connect(btnChart2, &QPushButton::clicked, this, [this]() {
        currentChartMode = 2;
        updateExpenseChart();
    });

    connect(btnChart3, &QPushButton::clicked, this, [this]() {
        currentChartMode = 3;
        updateIncomeChart();
    });



    QChartView *chartView = new QChartView(this);
    chartManager = new FinanceChartManager(chartView);
    mainChart = chartView->chart();

    dashboardLayout->addLayout(chartMenuLayout, 1);
    dashboardLayout->addWidget(chartView, 4);

    ui->stackedWidget->addWidget(dashboardWidget);

    addTransWidget = new AddTransaction(this);
    ui->stackedWidget->addWidget(addTransWidget);
    connect(addTransWidget, &AddTransaction::transactionAdded, this, &MainWindow::refreshTable);

    addCatWidget = new AddCategory(this);
    ui->stackedWidget->addWidget(addCatWidget);
    connect(addCatWidget, &AddCategory::categoryAdded, addTransWidget, &AddTransaction::loadCategories);

    model = new QSqlQueryModel(this);
    refreshTable();

    ui->tableView->setModel(model);
    ui->tableView->hideColumn(0);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    model->setHeaderData(1, Qt::Horizontal, "Kwota");
    model->setHeaderData(2, Qt::Horizontal, "Typ");
    model->setHeaderData(3, Qt::Horizontal, "Kategoria");
    model->setHeaderData(4, Qt::Horizontal, "Data");
    model->setHeaderData(5, Qt::Horizontal, "Opis");
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::onbtnDeleteclicked);
    ui->stackedWidget->setCurrentWidget(dashboardWidget);
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

    if (currentChartMode == 1) {
        updateChart();
    } else if (currentChartMode == 2) {
        updateExpenseChart();
    } else if (currentChartMode == 3) {
        updateIncomeChart();
    }
}

void MainWindow::updateChart()
{
    if (chartManager && model){
        chartManager->updateMainChart(model);
    }
}


void MainWindow::updateExpenseChart()
{
    if (chartManager) {
        chartManager->updateExpenseChart();
    }
}

void MainWindow::updateIncomeChart()
{
    if (chartManager) {
        chartManager->updateIncomeChart();
    }
}
void MainWindow::on_pushButton_clicked()
{
    mainChart->setAnimationOptions(QChart::NoAnimation);
    ui->stackedWidget->setCurrentWidget(addTransWidget);
}

void MainWindow::on_pushButton_2_clicked()
{
    mainChart->setAnimationOptions(QChart::NoAnimation);
    ui->stackedWidget->setCurrentWidget(addCatWidget);
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->stackedWidget->setCurrentWidget(dashboardWidget);
}
void MainWindow::onbtnDeleteclicked() {
    // 1. Sprawdzamy, czy tabela ma zaznaczony wiersz
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Uwaga", "Wybierz wiersz do usunięcia!");
        return;
    }

    if (ui->tableView->model() == transactionModel) {
        transactionModel->removeRow(currentIndex.row());
        transactionModel->submitAll();
        transactionModel->select();
    }

    else {
        int id = model->data(model->index(currentIndex.row(), 0)).toInt();
        QSqlQuery query;
        query.prepare("DELETE FROM transactions WHERE id = :id");
        query.bindValue(":id", id);

        if (query.exec()) {
            refreshTable();
        } else {
            QMessageBox::critical(this, "Błąd", "Nie udało się usunąć zapisu z bazy.");
        }
    }
}
// MOŻNA ROZDZIELIĆ TE WYKRESY NA OSOBNĄ KLASĘ