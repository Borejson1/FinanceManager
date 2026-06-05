#ifndef FINANCECHARTMANAGER_H
#define FINANCECHARTMANAGER_H

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtSql/QSqlQueryModel>
#include <QList>

class FinanceChartManager
{
public:
    FinanceChartManager(QChartView *chartView);
    ~FinanceChartManager();

    void updateMainChart(QSqlQueryModel *model);
    void updateExpenseChart();
    void updateIncomeChart();
    void updateMonthlySummaryChart();
    void updateYearlySummaryChart();
    QList<int> getAvailableYears();

private:
    QChartView *m_chartView;
    QChart *m_chart;
};

#endif