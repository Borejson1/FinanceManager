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

    ui->editDate->setPlaceholderText("DD-MM-RRRR");
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

void AddTransaction::on_editDate_textEdited(const QString &text)
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

void AddTransaction::on_pushButton_clicked()
{
    QString kwota = ui->editAmount->text();
    kwota.replace(',', '.');

    if (kwota.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Pole kwoty jest puste.");
        return;
    }

    QString opis = ui->lineEdit->text();
    QString wpisanaData = ui->editDate->text();

    if (wpisanaData.length() < 10) {
        QMessageBox::warning(this, "Błąd", "Niekompletna data.");
        return;
    }

    QDate date = QDate::fromString(wpisanaData, "dd-MM-yyyy");
    if (!date.isValid()) {
        QMessageBox::warning(this, "Błąd", "Nieprawidłowa data.");
        return;
    }

    QString dataDoBazy = date.toString("yyyy-MM-dd");
    QString czasUtworzenia = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    int categoryId = ui->comboType->currentData().toInt();

    if (categoryId <= 0) {
        QMessageBox::critical(this, "Błąd", "Nie wybrano poprawnej kategorii!");
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
        ui->editAmount->clear();
        ui->lineEdit->clear();
        ui->editDate->clear();

        emit transactionAdded();

        QMessageBox::information(this, "Sukces", "Transakcja została dodana!");
    } else {
        QMessageBox::critical(this, "Błąd", "Błąd zapisu: " + insertTrans.lastError().text());
    }
}