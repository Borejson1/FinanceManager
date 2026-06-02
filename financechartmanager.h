#ifndef FINANCECHARTMANAGER_H
#define FINANCECHARTMANAGER_H

void updateExpenseChart();
void updateIncomeChart();
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>

class FinanceChartManager
{
public:
    void updateMonthlySummaryChart();
    void updateYearlySummaryChart();

    FinanceChartManager(QChartView *chartView);
    ~FinanceChartManager();

    void updateMainChart(QSqlQueryModel *model);
    void updateExpenseChart();
    void updateIncomeChart();

private:
    QChartView *m_chartView;
    QChart *m_chart;
};

#endif // FINANCECHARTMANAGER_H