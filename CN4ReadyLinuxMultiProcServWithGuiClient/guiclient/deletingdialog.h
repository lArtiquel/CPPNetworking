#ifndef DELETINGDIALOG_H
#define DELETINGDIALOG_H

#include <QDialog>

namespace Ui {
class DeletingDialog;
}

class DeletingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeletingDialog(QWidget *parent = nullptr);
    int getNumberOfStudent() const;
    ~DeletingDialog();

private:
    Ui::DeletingDialog *ui;
};

#endif // DELETINGDIALOG_H
