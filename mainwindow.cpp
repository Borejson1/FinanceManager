#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QDebug>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    resize(1200, 800);

    ui->editDate->setPlaceholderText("DD-MM-RRRR");
    ui->editAmount->setValidator(new QDoubleValidator(0.0, 9999999.99, 2, this));

    QDir dir;
    if (!dir.exists("databases")) {
        dir.mkpath("databases");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/macie/Documents/FinanceManager/databases/finances.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Błąd bazy danych", "Nie udało się połączyć z bazą danych: " + db.lastError().text());
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS categories ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT NOT NULL, "
               "type TEXT NOT NULL)");

    query.exec("CREATE TABLE IF NOT EXISTS transactions ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "amount REAL NOT NULL, "
               "date TEXT NOT NULL, "
               "description TEXT, "
               "category_id INTEGER, "
               "created_at TEXT)");

    ui->comboType->clear();
    QSqlQuery catQuery("SELECT id, name, type FROM categories");
    while (catQuery.next()) {
        int id = catQuery.value(0).toInt();
        QString name = catQuery.value(1).toString();
        QString type = catQuery.value(2).toString();

        ui->comboType->addItem(name + " (" + type + ")", id);
    }

    model = new QSqlQueryModel(this);
    model->setQuery("SELECT t.id, t.amount, c.type, c.name, t.date, t.description "
                    "FROM transactions t "
                    "LEFT JOIN categories c ON t.category_id = c.id");

    ui->tableView->setModel(model);
    ui->tableView->hideColumn(0);

    model->setHeaderData(1, Qt::Horizontal, "Kwota");
    model->setHeaderData(2, Qt::Horizontal, "Typ");
    model->setHeaderData(3, Qt::Horizontal, "Kategoria");
    model->setHeaderData(4, Qt::Horizontal, "Data");
    model->setHeaderData(5, Qt::Horizontal, "Opis");

    ui->tableView->horizontalHeader()->setStretchLastSection(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_editDate_textEdited(const QString &text)
{
    QString cleanText;
    for (QChar c : text) {
        if (c.isDigit()) {
            cleanText += c;
        }
    }

    cleanText.truncate(8);

    QString formattedText;
    for (int i = 0; i < cleanText.length(); ++i) {
        if (i == 2 || i == 4) {
            formattedText += "-";
        }
        formattedText += cleanText[i];
    }

    ui->editDate->setText(formattedText);
}

void MainWindow::on_pushButton_clicked()
{
    QString kwota = ui->editAmount->text();
    kwota.replace(',', '.');

    if (kwota.isEmpty()) {
        QMessageBox::warning(this, "Błąd wprowadzania", "Pole kwoty jest puste. Proszę wpisać kwotę.");
        return;
    }

    QString opis = ui->lineEdit->text();
    QString wpisanaData = ui->editDate->text();

    if (wpisanaData.isEmpty()) {
        QMessageBox::warning(this, "Błąd wprowadzania", "Pole daty jest puste. Proszę wpisać datę.");
        return;
    }

    if (wpisanaData.length() < 10) {
        QMessageBox::warning(this, "Błąd wprowadzania", "Data jest niekompletna. Proszę wpisać pełną datę w formacie DD-MM-RRRR.");
        return;
    }

    QDate date = QDate::fromString(wpisanaData, "dd-MM-yyyy");

    if (!date.isValid()) {
        QMessageBox::warning(this, "Błąd wprowadzania", "Nieprawidłowa data. Proszę wpisać poprawną datę z kalendarza.");
        return;
    }

    QString dataDoBazy = date.toString("yyyy-MM-dd");
    QString czasUtworzenia = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    int categoryId = ui->comboType->currentData().toInt();

    if (categoryId <= 0) {
        QMessageBox::critical(this, "Błąd bazy danych", "Nie wybrano poprawnej kategorii!");
        return;
    }

    QSqlQuery insertTrans;
    insertTrans.prepare("INSERT INTO transactions (amount, date, description, category_id, created_at) "
                        "VALUES (?, ?, ?, ?, ?)");

    insertTrans.addBindValue(kwota.toDouble());
    insertTrans.addBindValue(dataDoBazy);
    insertTrans.addBindValue(opis);
    insertTrans.addBindValue(categoryId);
    insertTrans.addBindValue(czasUtworzenia);

    if(insertTrans.exec()) {
        model->setQuery("SELECT t.id, t.amount, c.type, c.name, t.date, t.description "
                        "FROM transactions t "
                        "LEFT JOIN categories c ON t.category_id = c.id");

        ui->editAmount->clear();
        ui->lineEdit->clear();
        ui->editDate->clear();
    } else {
        QMessageBox::critical(this, "Błąd bazy danych", "Błąd zapisu transakcji: " + insertTrans.lastError().text());
    }
}