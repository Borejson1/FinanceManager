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
    db.setDatabaseName("C:/Users/macie/Documents/FinanceManager/databases/finances.db");
    //DO POPRAWIENIA W DALSZYM CIĄGU!!!

    if (!db.open()) {
        QMessageBox::critical(this, "Błąd", "Nie udało się połączyć: " + db.lastError().text());
        return;
    }

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

    mainChart = new QChart();
    mainChart->setTitle("Podsumowanie Finansów");
    mainChart->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *chartView = new QChartView(mainChart);
    chartView->setRenderHint(QPainter::Antialiasing);

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
    model->setHeaderData(1, Qt::Horizontal, "Kwota");
    model->setHeaderData(2, Qt::Horizontal, "Typ");
    model->setHeaderData(3, Qt::Horizontal, "Kategoria");
    model->setHeaderData(4, Qt::Horizontal, "Data");
    model->setHeaderData(5, Qt::Horizontal, "Opis");
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

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
    mainChart->removeAllSeries();
    for (auto axis : mainChart->axes()) {
        mainChart->removeAxis(axis);
    }

    QSqlQuery queryIncome;
    queryIncome.exec("SELECT SUM(t.amount) "
                     "FROM transactions t "
                     "JOIN categories c ON t.category_id = c.id "
                     "WHERE c.type = 'Przychód'");

    double totalIncome = 0;
    if (queryIncome.next()) {
        totalIncome = queryIncome.value(0).toDouble();
    }

    QSqlQuery queryExpense;
    queryExpense.exec("SELECT SUM(t.amount) "
                      "FROM transactions t "
                      "JOIN categories c ON t.category_id = c.id "
                      "WHERE c.type = 'Wydatek'");

    double totalExpense = 0;
    if (queryExpense.next()) {
        totalExpense = queryExpense.value(0).toDouble();
    }

    QBarSet *incomeSet = new QBarSet("Przychody");
    QBarSet *expenseSet = new QBarSet("Wydatki");

    incomeSet->setColor(QColor("#2ecc71"));
    expenseSet->setColor(QColor("#e74c3c"));
    //TO NIE JEST BŁĄD ALE ZAWSZE MOŻNA TAK OGARNĄĆ ABY TEGO NIE BYŁO

    *incomeSet << totalIncome;
    *expenseSet << totalExpense;

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(incomeSet);
    barSeries->append(expenseSet);

    mainChart->addSeries(barSeries);
    mainChart->setTitle("Całkowity Bilans");

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append("Bilans");
    mainChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    double maxVal = std::max(totalIncome, totalExpense);
    axisY->setRange(0, maxVal * 1.2);
    axisY->setTitleText("Kwota w zł");
    mainChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
}

void MainWindow::updateExpenseChart()
{
    mainChart->removeAllSeries();
    for (auto axis : mainChart->axes()) {
        mainChart->removeAxis(axis);
    }

    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->setPieSize(0.7);

    QSqlQuery query;
    query.exec("SELECT c.name, SUM(t.amount) as total "
               "FROM transactions t "
               "JOIN categories c ON t.category_id = c.id "
               "WHERE c.type = 'Wydatek' "
               "GROUP BY c.id "
               "ORDER BY total DESC "
               "LIMIT 5");

    while (query.next()) {
        QString categoryName = query.value(0).toString();
        double amount = query.value(1).toDouble();
        pieSeries->append(categoryName, amount);
    }

    pieSeries->setLabelsVisible(true);
    for(QPieSlice *slice : pieSeries->slices()) {
        slice->setLabel(QString("%1: %2 zł").arg(slice->label()).arg(slice->value()));
        slice->setLabelPosition(QPieSlice::LabelOutside);
        if (slice->percentage() < 0.05) {
            slice->setLabelVisible(false);
        }
    }

    mainChart->addSeries(pieSeries);
    mainChart->setTitle("Top 5 Wydatków wg Kategorii");
}

void MainWindow::updateIncomeChart()
{
    mainChart->removeAllSeries();
    for (auto axis : mainChart->axes()) {
        mainChart->removeAxis(axis);
    }

    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->setPieSize(0.7);

    QSqlQuery query;
    query.exec("SELECT c.name, SUM(t.amount) as total "
               "FROM transactions t "
               "JOIN categories c ON t.category_id = c.id "
               "WHERE c.type = 'Przychód' "
               "GROUP BY c.id "
               "ORDER BY total DESC "
               "LIMIT 5");

    while (query.next()) {
        QString categoryName = query.value(0).toString();
        double amount = query.value(1).toDouble();
        pieSeries->append(categoryName, amount);
    }

    pieSeries->setLabelsVisible(true);
    for(QPieSlice *slice : pieSeries->slices()) {
        slice->setLabel(QString("%1: %2 zł").arg(slice->label()).arg(slice->value()));
        slice->setLabelPosition(QPieSlice::LabelOutside);
        if (slice->percentage() < 0.05) {
            slice->setLabelVisible(false);
        }
    }

    mainChart->addSeries(pieSeries);
    mainChart->setTitle("Top 5 Przychodów wg Kategorii");
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
// MOŻNA ROZDZIELIĆ TE WYKRESY NA OSOBNĄ KLASĘ