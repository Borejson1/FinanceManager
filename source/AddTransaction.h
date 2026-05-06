#ifndef ADDTRANSACTION_H
#define ADDTRANSACTION_H

#include <QWidget>

namespace Ui {
class AddTransaction;
}

class AddTransaction : public QWidget
{
    Q_OBJECT

public:
    explicit AddTransaction(QWidget *parent = nullptr);
    ~AddTransaction();

    void loadCategories();

signals:
    void transactionAdded();

private slots:
    void on_pushButton_clicked();
    void on_editDate_textEdited(const QString &text);

private:
    Ui::AddTransaction *ui;
};

#endif