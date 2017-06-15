#include <QDataStream>
#include "usernamesettingwidget.h"
#include "ui_usernamesettingwidget.h"
#include "socketthread.h"

UserNameSettingWidget::UserNameSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserNameSettingWidget)
{
    ui->setupUi(this);
    pSender = new SocketThread();
    connect(pSender, SIGNAL(connectedToServer()),
            this, SLOT(slotConnected()));
    connect(pSender, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotDataReady(QByteArray)));
    connect(pSender, SIGNAL(connectionError(QString)),
            this, SLOT(slotError(QString)));
    connect(this, SIGNAL(disconnectFromServer()),
            pSender, SLOT(slotDisconnectFromServer()));
    connect(pSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    connect(this, SIGNAL(connectToServer()),
            pSender, SLOT(slotConnectToServer()));
    connect(this, SIGNAL(sendString(QString)),
            pSender, SLOT(slotSendString(QString)));
    pSender->setDescription("<username>");
    emit connectToServer();
}

UserNameSettingWidget::~UserNameSettingWidget()
{
    delete ui;
    pSender->deleteLater();
}

void UserNameSettingWidget::slotConnected() {
    accepted = false;
    ui->errorsEdit->setText("Connection recieved.");
    ui->cmdOk->setText("&OK");
    ui->cmdOk->setEnabled(true);
    ui->nicknameEdit->setEnabled(true);
    disconnect(ui->cmdOk, SIGNAL(clicked()),
               pSender, SLOT(slotConnectToServer()));
    connect(ui->cmdOk, SIGNAL(clicked()),
            this, SLOT(slotOkClicked()));
    connect(ui->nicknameEdit, SIGNAL(returnPressed()),
            this, SLOT(slotOkClicked()));
}

void UserNameSettingWidget::slotOkClicked() {
    if (!ui->nicknameEdit->text().isEmpty()) {
        ui->cmdOk->setEnabled(false);
        ui->nicknameEdit->setEnabled(false);
        emit sendString(ui->nicknameEdit->text());
    }
}

void UserNameSettingWidget::slotDataReady(QByteArray data) {
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> accepted;
    if (accepted) {
        emit usernameAccepted(ui->nicknameEdit->text());
        ui->errorsEdit->setText("");
    }
    else {
        ui->errorsEdit->setText("Nickname " + ui->nicknameEdit->text() + " is already used.");
        ui->nicknameEdit->setText("");
    }
    ui->cmdOk->setEnabled(true);
    ui->nicknameEdit->setEnabled(true);
}

void UserNameSettingWidget::slotDisconnected() {
    ui->errorsEdit->setText("Disconnected from server.");
    disconnect(ui->cmdOk, SIGNAL(clicked()),
            this, SLOT(slotOkClicked()));
    connect(ui->cmdOk, SIGNAL(clicked()),
            pSender, SLOT(slotConnectToServer()));
    ui->cmdOk->setText("&Reconnect");
    ui->cmdOk->setEnabled(true);
    ui->nicknameEdit->setEnabled(false);
}

void UserNameSettingWidget::slotError(const QString &strError) {
    ui->errorsEdit->setText(strError);
    if (pSender->isConnected()) {
        emit disconnectFromServer();
        ui->cmdOk->setEnabled(false);
        ui->nicknameEdit->setEnabled(false);
    }
    else {
        disconnect(ui->cmdOk, SIGNAL(clicked()),
                this, SLOT(slotOkClicked()));
        connect(ui->cmdOk, SIGNAL(clicked()),
                pSender, SLOT(slotConnectToServer()));
        ui->cmdOk->setText("&Reconnect");
        ui->cmdOk->setEnabled(true);
        ui->nicknameEdit->setEnabled(false);
    }
}
