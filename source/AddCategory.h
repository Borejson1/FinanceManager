#ifndef ADDCATEGORY_H
#define ADDCATEGORY_H

#include <QWidget>

namespace Ui {
class AddCategory;
}

class AddCategory : public QWidget
{
    Q_OBJECT

public:
    explicit AddCategory(QWidget *parent = nullptr);
    ~AddCategory();

signals:
    void categoryAdded();

private slots:
    void on_pushButton_clicked();

private:
    Ui::AddCategory *ui;
};

#endif