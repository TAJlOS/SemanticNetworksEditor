#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setStyleSheet("Dialog {background-color:white}");
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    emit findPath(ui->editBegin->text(),ui->editEnd->text(),ui->radioDir->isChecked());
}

void Dialog::setOutput(QString text)
{
    ui->editOutput->setText(text);
}
