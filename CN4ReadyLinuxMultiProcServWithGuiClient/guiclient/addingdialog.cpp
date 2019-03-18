#include "addingdialog.h"
#include "ui_addingdialog.h"

addingDialog::addingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addingDialog)
{
    ui->setupUi(this);
}

QString addingDialog::getName()
{
    return ui->nameEdit->text();
}

QString addingDialog::getSurname()
{
    return ui->surnameEdit->text();
}

int addingDialog::getMark()
{
    return ui->markEdit->text().toInt();
}

addingDialog::~addingDialog()
{
    delete ui;
}
