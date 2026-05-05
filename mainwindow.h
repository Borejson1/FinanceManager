#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQueryModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_editDate_textEdited(const QString &text);
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QSqlQueryModel *model;
};
#endif // MAINWINDOW_H