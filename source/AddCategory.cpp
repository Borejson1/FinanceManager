#include "AddCategory.h"
#include "ui_AddCategory.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

AddCategory::AddCategory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddCategory)
{
    ui->setupUi(this);
}

AddCategory::~AddCategory()
{
    delete ui;
}

void AddCategory::on_pushButton_clicked()
{
    QString nazwaKategorii = ui->lineEdit->text().trimmed();

    if (nazwaKategorii.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Nazwa kategorii nie może być pusta!");
        return;
    }

    QString typKategorii;
    if (ui->radioIncome->isChecked()) {
        typKategorii = "Przychód";
    } else {
        typKategorii = "Wydatek";
    }

    QSqlQuery query;
    query.prepare("INSERT INTO categories (name, type) VALUES (?, ?)");
    query.addBindValue(nazwaKategorii);
    query.addBindValue(typKategorii);

    if (query.exec()) {
        ui->lineEdit->clear();

        emit categoryAdded();

        QMessageBox::information(this, "Sukces", "Kategoria '" + nazwaKategorii + "' została dodana!");
    } else {
        QMessageBox::critical(this, "Błąd bazy danych", "Nie udało się dodać kategorii: " + query.lastError().text());
    }
}