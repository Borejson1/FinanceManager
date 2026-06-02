#include "mainwindow.h"
#include "financechartmanager.h"
#include "ui_mainwindow.h"
#include "source/AddTransaction.h"
#include "source/AddCategory.h"
#include "edittransactiondialog.h"

#include <QCoreApplication>
#include <QDir>
#include <QTableWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
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
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1200, 800);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    currentChartMode = 1;

    QString appPath = QCoreApplication::applicationDirPath();
    QDir dir(appPath);

    if (!dir.exists("databases")) {
        dir.mkpath("databases");
    }

    QString localDbPath = dir.absoluteFilePath("databases/finances.db");
    QString oldDbPath = "C:/Users/macie/Documents/FinanceManager/databases/finances.db";

    if (!QFile::exists(localDbPath)) {
        if (QFile::exists(oldDbPath)) {
            QFile::copy(oldDbPath, localDbPath);
            QFile::setPermissions(localDbPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(localDbPath);

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
    QPushButton *btnChart4 = new QPushButton("Podsumowanie Miesięczne", this);
    QPushButton *btnChart5 = new QPushButton("Podsumowanie Roczne", this);

    chartMenuLayout->addWidget(btnChart1);
    chartMenuLayout->addWidget(btnChart2);
    chartMenuLayout->addWidget(btnChart3);
    chartMenuLayout->addWidget(btnChart4);
    chartMenuLayout->addWidget(btnChart5);
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

    connect(btnChart4, &QPushButton::clicked, this, [this]() {
        currentChartMode = 4;
        if (chartManager) chartManager->updateMonthlySummaryChart();
    });

    connect(btnChart5, &QPushButton::clicked, this, [this]() {
        currentChartMode = 5;
        if (chartManager) chartManager->updateYearlySummaryChart();
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

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterKeyColumn(-1);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tableView->setModel(proxyModel);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(6);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    model->setHeaderData(1, Qt::Horizontal, "Kwota");
    model->setHeaderData(2, Qt::Horizontal, "Typ");
    model->setHeaderData(3, Qt::Horizontal, "Kategoria");
    model->setHeaderData(4, Qt::Horizontal, "Data");
    model->setHeaderData(5, Qt::Horizontal, "Opis");

    ui->tableView->setColumnWidth(3, 220);
    ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);

    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::onbtnDeleteclicked);
    ui->stackedWidget->setCurrentWidget(dashboardWidget);
    connect(ui->lineEditFilter, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString);

    updateChart();
}

MainWindow::~MainWindow()
{
    delete ui;
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
        QModelIndex sourceIndex = proxyModel->mapToSource(currentIndex);
        int id = model->data(model->index(sourceIndex.row(), 0)).toInt();

        QSqlQuery query;
        query.prepare("DELETE FROM transactions WHERE id = :id");
        query.bindValue(":id", id);

        if (query.exec()) {
            refreshTable();
        } else {
            QMessageBox::critical(this, "Błąd", "Nie udało się usunąć zapisu z bazy.");
        }

        QModelIndex proxyIndex = ui->tableView->currentIndex();
        if (!proxyIndex.isValid()) {
            return;
        }

        sourceIndex = proxyModel->mapToSource(proxyIndex);
        int row = sourceIndex.row();

        id = model->data(model->index(row, 0)).toInt();
        double amount = model->data(model->index(row, 1)).toDouble();
        QString dateStr = model->data(model->index(row, 4)).toString();
        QString desc = model->data(model->index(row, 5)).toString();
        int catId = model->data(model->index(row, 6)).toInt();

        EditTransactionDialog dialog(this);
        dialog.loadCategories();
        dialog.setTransactionData(amount, QDate::fromString(dateStr, "yyyy-MM-dd"), desc, catId);

        if (dialog.exec() == QDialog::Accepted) {
            QSqlQuery query;
            query.prepare("UPDATE transactions SET amount = :amt, date = :date, "
                          "category_id = :cat, description = :desc WHERE id = :id");
            query.bindValue(":amt", dialog.getAmount());
            query.bindValue(":date", dialog.getDate().toString("yyyy-MM-dd"));
            query.bindValue(":cat", dialog.getSelectedCategoryId());
            query.bindValue(":desc", dialog.getDescription());
            query.bindValue(":id", id);

            if (query.exec()) {
                refreshTable();
            } else {
                QMessageBox::critical(this, "Błąd", "Nie udało się zaktualizować danych.");
            }
        }
    }
}

void MainWindow::on_btnEdit_clicked()
{
    QModelIndex proxyIndex = ui->tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this, "Uwaga", "Wybierz wiersz do edycji!");
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    int row = sourceIndex.row();

    int id = model->data(model->index(row, 0)).toInt();
    double amount = model->data(model->index(row, 1)).toDouble();
    QString dateStr = model->data(model->index(row, 4)).toString();
    QString desc = model->data(model->index(row, 5)).toString();
    int catId = model->data(model->index(row, 6)).toInt();

    EditTransactionDialog dialog(this);
    dialog.loadCategories();
    dialog.setTransactionData(amount, QDate::fromString(dateStr, "yyyy-MM-dd"), desc, catId);

    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery query;
        query.prepare("UPDATE transactions SET amount = :amt, date = :date, "
                      "category_id = :cat, description = :desc WHERE id = :id");
        query.bindValue(":amt", dialog.getAmount());
        query.bindValue(":date", dialog.getDate().toString("yyyy-MM-dd"));
        query.bindValue(":cat", dialog.getSelectedCategoryId());
        query.bindValue(":desc", dialog.getDescription());
        query.bindValue(":id", id);

        if (query.exec()) {
            refreshTable();
            QMessageBox::information(this, "Sukces", "Dane zostały pomyślnie zaktualizowane!");
        } else {
            QMessageBox::critical(this, "Błąd", "Nie udało się zaktualizować danych.");
        }
    }
}

double MainWindow::calculateBalance()
{
    QSqlQuery query("SELECT SUM(CASE WHEN c.type = 'Przychód' THEN t.amount ELSE -t.amount END) "
                    "FROM transactions t "
                    "LEFT JOIN categories c ON t.category_id = c.id");

    if (query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

void MainWindow::refreshTable()
{
    double balance = calculateBalance();
    model->setQuery("SELECT t.id, t.amount, c.type, c.name, t.date, t.description, t.category_id "
                    "FROM transactions t "
                    "LEFT JOIN categories c ON t.category_id = c.id");

    QString color = (balance >= 0) ? "green" : "red";
    ui->lblBalance->setText(QString("<b style='color:%1;'>Saldo: %2 zł</b>").arg(color).arg(balance, 0, 'f', 2));
}

void MainWindow::updateChart() {
    if (chartManager && model){
        chartManager->updateMainChart(model);
    }
}