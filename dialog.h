#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTextEdit>
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void setOutput(QString text);
signals:
    void findPath(QString begin,QString end, bool);
private slots:
    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
