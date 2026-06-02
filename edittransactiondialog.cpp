#include "edittransactiondialog.h"
#include "ui_edittransactiondialog.h"
#include <QSqlQuery>

EditTransactionDialog::EditTransactionDialog(QWidget *parent) : QDialog(parent), ui(new Ui::EditTransactionDialog) {
    ui->setupUi(this);
    ui->comboCategory->setMaxVisibleItems(4);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void EditTransactionDialog::loadCategories() {
    ui->comboCategory->clear();
    QSqlQuery query("SELECT id, name FROM categories");
    while (query.next()) {
        ui->comboCategory->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void EditTransactionDialog::setTransactionData(double amount, QDate date, QString desc, int catId) {
    ui->spinAmount->setValue(amount);
    ui->dateEdit->setDate(date);
    ui->editDescription->setText(desc);
    int index = ui->comboCategory->findData(catId);
    ui->comboCategory->setCurrentIndex(index);
}

double EditTransactionDialog::getAmount() const {
    return ui->spinAmount->value();
}

QDate EditTransactionDialog::getDate() const {
    return ui->dateEdit->date();
}

QString EditTransactionDialog::getDescription() const {
    return ui->editDescription->text();
}

int EditTransactionDialog::getSelectedCategoryId() const {
    return ui->comboCategory->currentData().toInt();
}

EditTransactionDialog::~EditTransactionDialog() {
    delete ui;
}