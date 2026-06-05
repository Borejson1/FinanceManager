#include <QDialog>
#include <QDate>

namespace Ui { class EditTransactionDialog; }

class EditTransactionDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditTransactionDialog(QWidget *parent = nullptr);
    ~EditTransactionDialog();

    void setTransactionData(double amount, QDate date, QString desc, int catId);
    double getAmount() const;
    QDate getDate() const;
    QString getDescription() const;
    int getSelectedCategoryId() const;
    void loadCategories();

private:
    Ui::EditTransactionDialog *ui;
};
