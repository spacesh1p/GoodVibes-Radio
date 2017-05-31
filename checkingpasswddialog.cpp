#include "checkingpasswddialog.h"
#include "ui_checkingpasswddialog.h"

CheckingPasswdDialog::CheckingPasswdDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CheckingPasswdDialog)
{
    ui->setupUi(this);
}

CheckingPasswdDialog::~CheckingPasswdDialog()
{
    delete ui;
}

QString CheckingPasswdDialog::getPasswdLine() {
    QString passwd = ui->passwdLine->text();
    ui->passwdLine->setText("");
    return passwd;
}

