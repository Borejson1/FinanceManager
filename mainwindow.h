#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/Qchart>
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QSqlTableModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class AddTransaction;
class AddCategory;
class FinanceChartManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void refreshTable();
    void updateChart();
    void updateExpenseChart();
    void updateIncomeChart();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void onbtnDeleteclicked();

private:
    QSqlTableModel *transactionModel;
    Ui::MainWindow *ui;
    QSqlQueryModel *model;

    FinanceChartManager *chartManager;

    QWidget *dashboardWidget;
    AddTransaction *addTransWidget;
    AddCategory *addCatWidget;

    QChart *mainChart;
    int currentChartMode;
};
#endif