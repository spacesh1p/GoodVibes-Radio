#ifndef CHECKINGPASSWDDIALOG_H
#define CHECKINGPASSWDDIALOG_H

#include <QDialog>

namespace Ui {
class CheckingPasswdDialog;
}

class CheckingPasswdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckingPasswdDialog(QWidget *parent = 0);
    ~CheckingPasswdDialog();
    QString getPasswdLine();

private:
    Ui::CheckingPasswdDialog *ui;
};

#endif // CHECKINGPASSWDDIALOG_H
