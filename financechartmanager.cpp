#include "financechartmanager.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtGui/QPainter>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

FinanceChartManager::FinanceChartManager(QChartView *chartView)
    : m_chartView(chartView)
{
    m_chart = new QChart();
    m_chart->setTitle("Całkowity Bilans");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

FinanceChartManager::~FinanceChartManager()
{

}

void FinanceChartManager::updateMainChart(QSqlQueryModel *model)
{
    m_chart->removeAllSeries();
    for (auto axis : m_chart->axes()) {
        m_chart->removeAxis(axis);
    }

    QBarSet *setPrzychody = new QBarSet("Przychody");
    QBarSet *setWydatki = new QBarSet("Wydatki");

    setPrzychody->setColor(QColor(46, 204, 113));
    setWydatki->setColor(QColor(231, 76, 60));

    double sumaPrzychodow = 0.0;
    double sumaWydatkow = 0.0;

    for (int i = 0; i < model->rowCount(); ++i) {

        double kwota = model->record(i).value("amount").toDouble();
        QString typ = model->record(i).value("type").toString();

        if (typ == "Przychód" || typ == "Income") {
            sumaPrzychodow += kwota;
        } else if (typ == "Wydatek" || typ == "Expense") {
            sumaWydatkow += kwota;
        }
    }

    *setPrzychody << sumaPrzychodow;
    *setWydatki << sumaWydatkow;

    QBarSeries *series = new QBarSeries();
    series->append(setPrzychody);
    series->append(setWydatki);
    m_chart->addSeries(series);

    QStringList categories;
    categories << "Bilans";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    m_chart->setAxisX(axisX, series);

    QValueAxis *axisY = new QValueAxis();
    double maxWartosc = std::max(sumaPrzychodow, sumaWydatkow) * 1.2;
    axisY->setRange(0, maxWartosc > 0 ? maxWartosc : 1000);
    axisY->setTitleText("Kwota w zł");
    m_chart->setAxisY(axisY, series);
}

void FinanceChartManager::updateExpenseChart()
{
    if (!m_chart) return;

    m_chart->removeAllSeries();
    for (auto axis : m_chart->axes()) {
        m_chart->removeAxis(axis);
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

    m_chart->addSeries(pieSeries);
    m_chart->setTitle("Top 5 Wydatków wg Kategorii");
}

void FinanceChartManager::updateIncomeChart()
{
    if (!m_chart) return;

    m_chart->removeAllSeries();
    for (auto axis : m_chart->axes()) {
        m_chart->removeAxis(axis);
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

    m_chart->addSeries(pieSeries);
    m_chart->setTitle("Top 5 Przychodów wg Kategorii");
}