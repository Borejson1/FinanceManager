#include "AddTransaction.h"
#include "source/ui_AddTransaction.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDateTime>
#include <QDoubleValidator>
#include <QMessageBox>

AddTransaction::AddTransaction(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddTransaction)
{
    ui->setupUi(this);
    ui->editDate->setDate(QDate::currentDate());
    ui->editDate->setCalendarPopup(true);
    ui->editDate->setDisplayFormat("dd-MM-yyyy");
    ui->editDate->setDate(QDate::currentDate());

    ui->editAmount->setValidator(new QDoubleValidator(0.0, 9999999.99, 2, this));

    loadCategories();
}

AddTransaction::~AddTransaction()
{
    delete ui;
}

void AddTransaction::loadCategories()
{
    ui->comboType->clear();
    QSqlQuery catQuery("SELECT id, name, type FROM categories");
    while (catQuery.next()) {
        int id = catQuery.value(0).toInt();
        QString name = catQuery.value(1).toString();
        QString type = catQuery.value(2).toString();

        ui->comboType->addItem(name + " (" + type + ")", id);
    }
}



void AddTransaction::on_pushButton_clicked()
{
    QString kwota = ui->editAmount->text();
    kwota.replace(',', '.');

    if (kwota.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Pole kwoty jest puste.");
        return;
    }

    QString opis = ui->lineEdit->text();
    QDate date = ui->editDate->date();
    QString dataDoBazy = date.toString("yyyy-MM-dd");

    QString czasUtworzenia = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    int categoryId = ui->comboType->currentData().toInt();

    if (categoryId <= 0) {
        QMessageBox::critical(this, "Błąd", "Nie wybrano poprawnej kategorii!");
        return;
    }

    QSqlQuery insertTrans;
    insertTrans.prepare("INSERT INTO transactions (amount, date, description, category_id, created_at, is_fixed) "
                        "VALUES (?, ?, ?, ?, ?, ?)");
    insertTrans.addBindValue(kwota.toDouble());
    insertTrans.addBindValue(dataDoBazy);
    insertTrans.addBindValue(opis);
    insertTrans.addBindValue(categoryId);
    insertTrans.addBindValue(czasUtworzenia);
    insertTrans.addBindValue(ui->checkIsFixed->isChecked() ? 1 : 0);

    if(insertTrans.exec()) {
        ui->editAmount->clear();
        ui->lineEdit->clear();
        ui->editDate->clear();

        emit transactionAdded();

        QMessageBox::information(this, "Sukces", "Transakcja została dodana!");
    } else {
        QMessageBox::critical(this, "Błąd", "Błąd zapisu: " + insertTrans.lastError().text());
    }
}