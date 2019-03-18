#include "deletingdialog.h"
#include "ui_deletingdialog.h"

DeletingDialog::DeletingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeletingDialog)
{
    ui->setupUi(this);
}

int DeletingDialog::getNumberOfStudent() const
{
    return ui->spinBox->value();
}

DeletingDialog::~DeletingDialog()
{
    delete ui;
}
