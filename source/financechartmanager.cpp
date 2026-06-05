#include "financechartmanager.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtGui/QPainter>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtCore/QDate>
#include <QtCore/QMap>
#include <algorithm>
#include <QList>

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
    m_chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    double maxWartosc = std::max(sumaPrzychodow, sumaWydatkow) * 1.2;
    axisY->setRange(0, maxWartosc > 0 ? maxWartosc : 1000);
    axisY->setTitleText("Kwota w zł");
    m_chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
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

    int currentYear = QDate::currentDate().year();

    QSqlQuery query;
    query.prepare("SELECT strftime('%m', t.date) as month, c.type, SUM(t.amount) "
                  "FROM transactions t JOIN categories c ON t.category_id = c.id "
                  "WHERE strftime('%Y', t.date) = :year "
                  "GROUP BY month, c.type");
    query.bindValue(":year", QString::number(currentYear));
    query.exec();

    while (query.next()) {
        int mIndex = query.value(0).toInt() - 1;
        if (mIndex >= 0 && mIndex < 12) {
            QString type = query.value(1).toString();
            double amt = query.value(2).toDouble();
            if (type == "Przychód" || type == "Income") przychodyM[mIndex] += amt;
            else if (type == "Wydatek" || type == "Expense") wydatkiM[mIndex] += amt;
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
    series->setBarWidth(0.8);
    m_chart->addSeries(series);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(months);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxWartosc > 0 ? maxWartosc * 1.2 : 1000);
    axisY->setTitleText("Kwota w zł");
    m_chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    m_chart->setTitle(QString("Podsumowanie Miesięczne (%1 Rok)").arg(currentYear));
}

void FinanceChartManager::updateYearlySummaryChart()
{
    if (!m_chart) return;

    m_chart->removeAllSeries();
    for (auto axis : m_chart->axes()) {
        m_chart->removeAxis(axis);
    }

    int currentYear = QDate::currentDate().year();
    QStringList yearsToShow;
    yearsToShow << QString::number(currentYear - 2)
                << QString::number(currentYear - 1)
                << QString::number(currentYear);

    QMap<QString, double> incomeData;
    QMap<QString, double> expenseData;

    QSqlQuery query;
    query.prepare("SELECT strftime('%Y', t.date) as year, c.type, SUM(t.amount) "
                  "FROM transactions t "
                  "JOIN categories c ON t.category_id = c.id "
                  "WHERE strftime('%Y', t.date) IN (:y1, :y2, :y3) "
                  "GROUP BY year, c.type "
                  "ORDER BY year ASC");
    query.bindValue(":y1", yearsToShow[0]);
    query.bindValue(":y2", yearsToShow[1]);
    query.bindValue(":y3", yearsToShow[2]);
    query.exec();

    while (query.next()) {
        QString year = query.value(0).toString();
        QString type = query.value(1).toString();
        double amount = query.value(2).toDouble();

        if (type == "Przychód" || type == "Income") {
            incomeData[year] += amount;
        } else if (type == "Wydatek" || type == "Expense") {
            expenseData[year] += amount;
        }
    }

    QBarSet *incomeSet = new QBarSet("Przychody");
    QBarSet *expenseSet = new QBarSet("Wydatki");
    incomeSet->setColor(QColor(46, 204, 113));
    expenseSet->setColor(QColor(231, 76, 60));

    double maxValue = 0;

    for (const QString &year : yearsToShow) {
        double inc = incomeData.value(year, 0.0);
        double exp = expenseData.value(year, 0.0);

        *incomeSet << inc;
        *expenseSet << exp;

        maxValue = std::max({maxValue, inc, exp});
    }

    QBarSeries *series = new QBarSeries();
    series->append(incomeSet);
    series->append(expenseSet);
    series->setBarWidth(0.6);
    m_chart->addSeries(series);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(yearsToShow);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxValue > 0 ? maxValue * 1.2 : 1000);
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText("Kwota w zł");
    m_chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    m_chart->setTitle(QString("Podsumowanie Roczne (%1 - %2)").arg(yearsToShow.first(), yearsToShow.last()));
}

QList<int> FinanceChartManager::getAvailableYears() {
    QList<int> years;
    QSqlQuery query("SELECT DISTINCT strftime('%Y', date) FROM transactions ORDER BY date DESC");
    while (query.next()) {
        years << query.value(0).toInt();
    }
    return years;
}