#ifndef ADDINGDIALOG_H
#define ADDINGDIALOG_H

#include <QDialog>

namespace Ui {
class addingDialog;
}

class addingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addingDialog(QWidget *parent = nullptr);
    QString getName();
    QString getSurname();
    int getMark();
    ~addingDialog();

private:
    Ui::addingDialog *ui;
};

#endif // ADDINGDIALOG_H
