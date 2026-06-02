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
#include <QtCore/QDate>
#include <QtCore/QMap>
#include <algorithm>

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

void FinanceChartManager::updateMonthlySummaryChart()
{
    if (!m_chart) return;

    m_chart->removeAllSeries();
    for (auto axis : m_chart->axes()) {
        m_chart->removeAxis(axis);
    }

    QBarSet *setPrzychody = new QBarSet("Przychody");
    QBarSet *setWydatki = new QBarSet("Wydatki");
    setPrzychody->setColor(QColor(46, 204, 113));
    setWydatki->setColor(QColor(231, 76, 60));

    QStringList months = {"Sty", "Lut", "Mar", "Kwi", "Maj", "Cze", "Lip", "Sie", "Wrz", "Paź", "Lis", "Gru"};
    double przychodyM[12] = {0};
    double wydatkiM[12] = {0};

    QSqlQuery query;
    query.exec("SELECT strftime('%m', t.date) as month, c.type, SUM(t.amount) "
               "FROM transactions t JOIN categories c ON t.category_id = c.id "
               "WHERE strftime('%Y', t.date) = strftime('%Y', 'now') "
               "GROUP BY month, c.type");

    while (query.next()) {
        int mIndex = query.value(0).toInt() - 1;
        if (mIndex >= 0 && mIndex < 12) {
            QString type = query.value(1).toString();
            double amt = query.value(2).toDouble();
            if (type == "Przychód") przychodyM[mIndex] += amt;
            else if (type == "Wydatek") wydatkiM[mIndex] += amt;
        }
    }

    double maxWartosc = 0;
    for (int i = 0; i < 12; i++) {
        *setPrzychody << przychodyM[i];
        *setWydatki << wydatkiM[i];
        maxWartosc = std::max({maxWartosc, przychodyM[i], wydatkiM[i]});
    }

    QBarSeries *series = new QBarSeries();
    series->append(setPrzychody);
    series->append(setWydatki);
    m_chart->addSeries(series);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(months);
    m_chart->setAxisX(axisX, series);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxWartosc > 0 ? maxWartosc * 1.2 : 1000);
    axisY->setTitleText("Kwota w zł");
    m_chart->setAxisY(axisY, series);
    m_chart->setTitle("Podsumowanie Miesięczne (Obecny Rok)");
}

void FinanceChartManager::updateYearlySummaryChart()
{
    if (!m_chart) return;

    m_chart->removeAllSeries();
    for (auto axis : m_chart->axes()) {
        m_chart->removeAxis(axis);
    }

    QBarSet *setPrzychody = new QBarSet("Przychody");
    QBarSet *setWydatki = new QBarSet("Wydatki");
    setPrzychody->setColor(QColor(46, 204, 113));
    setWydatki->setColor(QColor(231, 76, 60));

    QSqlQuery query;
    QStringList years;
    query.exec("SELECT DISTINCT strftime('%Y', date) as year FROM transactions ORDER BY year");
    while (query.next()) {
        years << query.value(0).toString();
    }

    if (years.isEmpty()) {
        years << QDate::currentDate().toString("yyyy");
    }

    QMap<QString, double> przychodyY;
    QMap<QString, double> wydatkiY;

    query.exec("SELECT strftime('%Y', t.date) as year, c.type, SUM(t.amount) "
               "FROM transactions t JOIN categories c ON t.category_id = c.id "
               "GROUP BY year, c.type");

    while(query.next()) {
        QString year = query.value(0).toString();
        QString type = query.value(1).toString();
        double amt = query.value(2).toDouble();
        if (type == "Przychód") przychodyY[year] += amt;
        else if (type == "Wydatek") wydatkiY[year] += amt;
    }

    double maxWartosc = 0;
    for (const QString &y : years) {
        double p = przychodyY.value(y, 0.0);
        double w = wydatkiY.value(y, 0.0);
        *setPrzychody << p;
        *setWydatki << w;
        maxWartosc = std::max({maxWartosc, p, w});
    }

    QBarSeries *series = new QBarSeries();
    series->append(setPrzychody);
    series->append(setWydatki);
    m_chart->addSeries(series);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(years);
    m_chart->setAxisX(axisX, series);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxWartosc > 0 ? maxWartosc * 1.2 : 1000);
    axisY->setTitleText("Kwota w zł");
    m_chart->setAxisY(axisY, series);
    m_chart->setTitle("Podsumowanie Roczne");
}