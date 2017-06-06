#include <QDateTime>
#include <QCommandLinkButton>
#include "choosechannelwidget.h"
#include "ui_choosechannelwidget.h"
#include "channelwidget.h"
#include "channel.h"
#include "mainwindow.h"
#include "socketthread.h"
#include "playerwidget.h"
#include "checkingpasswddialog.h"

ChooseChannelWidget::ChooseChannelWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChooseChannelWidget)
{
    ui->setupUi(this);

    QString parentClass = parent->metaObject()->className();
    if(parentClass == "MainWindow") {                                     // if parent is Server
        pMainWindow = (MainWindow*)parent;
        pMainWindow->addWidget(this);
    }
    else
        pMainWindow = nullptr;

    userName = pMainWindow->getUserName();

    pPlayerWidget = nullptr;

    pCheckingPasswdDialog = new CheckingPasswdDialog(this);

    pReaderThread = new SocketThread();
    pReaderThread->setDescription(QString("<user:" + userName + ">,<purpose:readChannelsInfo>"));
    connect(this, SIGNAL(connectToServer()),
            pReaderThread, SLOT(slotConnectToServer()));
    connect(this, SIGNAL(disconnectFromServer()),
            pReaderThread, SLOT(slotDisconnectFromServer()));
    connect(this, SIGNAL(sendString(QString)),
            pReaderThread, SLOT(slotSendString(QString)));
    connect(pReaderThread, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotDataReady(QByteArray)));
    connect(pReaderThread, SIGNAL(connectionError(QString)),
            this, SLOT(slotConnectionError(QString)));
    connect(pReaderThread, SIGNAL(connectedToServer()),
            this, SLOT(slotConnected()));
    connect(pReaderThread, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    emit connectToServer();

    connect(ui->cmdChoose, SIGNAL(clicked()),                       // handle cmd Choose press
            this, SLOT(slotChooseClicked()));
    connect(ui->cmdCreateChannel, SIGNAL(clicked()),                // handle Create channel press
            this, SLOT(slotCreateChannel()));
}

ChooseChannelWidget::~ChooseChannelWidget()
{
    delete pReaderThread;
    delete ui;
}

QString ChooseChannelWidget::getUserName() {
    return userName;
}

void ChooseChannelWidget::slotCreateChannel() {
    ChannelWidget* pChannelWidget = new ChannelWidget(new Channel(userName), this);     // create new channel widget
    if (pChannelWidget->openSettingsDialog()) {                                         // open setting dialog window
        hostChannelsList.append(pChannelWidget);                                        // append it to the list
        for (auto it = guestButtons.begin(); it != guestButtons.end(); it++)
            (*it)->setEnabled(false);
        QCommandLinkButton* pChannelButton = new QCommandLinkButton((pChannelWidget->getChannel())->getChannelName());
        pChannelButton->setCheckable(true);
        pChannelButton->setAutoExclusive(true);
        pChannelButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        int row, col, rs, cs;
        ui->channelsLayout->getItemPosition((ui->channelsLayout)->indexOf(ui->hostLine), &row, &col, &rs, &cs);
        ui->channelsLayout->removeWidget(ui->hostLine);
        ui->channelsLayout->addWidget(pChannelButton, row, col, rs, cs);
        ui->channelsLayout->addWidget(ui->hostLine, row + 1, col, rs, cs);
        connect(pChannelButton, SIGNAL(toggled(bool)),
                this, SLOT(slotHostChannelClicked()));
        pChannelButton->setChecked(true);
        hostButtons.append(pChannelButton);
        pMainWindow->addWidget(pChannelWidget);
    }
    else {
        delete pChannelWidget;
    }
}

void ChooseChannelWidget::slotEditChannel() {
    ChannelWidget* pChannelWidget = pairChoosenHostChannel.first;
    Channel* pChannel = pChannelWidget->getChannel();
    if (pChannelWidget->openSettingsDialog()) {
        pChannelWidget->updateData();
        (pairChoosenHostChannel.second)->setText(pChannel->getChannelName());
        ui->textEdit->setText(pChannel->getChannelName() + "\n" +
                              "Maximum number of guest: " + QString::number(pChannel->getMaxGuestsNum()) + "\n" +
                              pChannel->getDescription());
    }
}

void ChooseChannelWidget::slotHostChannelClicked() {
    QCommandLinkButton* pButton = (QCommandLinkButton*)sender();
    if (pButton->isChecked()) {
        if (ui->cmdEdit->text() == "&Edit")
            ui->cmdEdit->setEnabled(true);                                                  // make cmd Edit enabled
        ui->cmdChoose->setEnabled(true);                                                    // make cmd Choose enabled
        ChannelWidget* pChannelWidget = getHostChannel(pButton->text());
        Channel* pChannel = pChannelWidget->getChannel();
        pairChoosenHostChannel = qMakePair(pChannelWidget, pButton);                        // write choosen channel
        pairChoosenGuestChannel = qMakePair(nullptr, nullptr);
        ui->textEdit->setText(pChannel->getChannelName() + "\n" +
                              "Maximum number of guest: " + QString::number(pChannel->getMaxGuestsNum()) + "\n" +
                              pChannel->getDescription());
    }
    else {
        if (ui->cmdEdit->text() == "&Edit")
            ui->cmdEdit->setEnabled(false);                                       // make cmd Edit disabled
        ui->cmdChoose->setEnabled(false);                                         // make cmd Choose disabled
        pairChoosenHostChannel = qMakePair(nullptr, nullptr);
        ui->textEdit->setText("Good Vibes Radio\n" + QDateTime::currentDateTime().toString("ddd, d MMMM HH:mm"));
    }
}

void ChooseChannelWidget::slotGuestChannelClicked() {
    QCommandLinkButton* pButton = (QCommandLinkButton*)sender();
    if (pButton->isChecked()) {
        if (ui->cmdEdit->text() == "&Edit")
            ui->cmdEdit->setEnabled(false);                                                 // make cmd Edit disabled
        ui->cmdChoose->setEnabled(true);                                                    // make cmd Choose enabled
        Channel* pChannel= getGuestChannel(pButton->text());
        pairChoosenGuestChannel = qMakePair(pChannel, pButton);                             // write choosen channel
        pairChoosenHostChannel = qMakePair(nullptr, nullptr);
        ui->textEdit->setText(pChannel->getChannelName() + "\n" +
                              "Maximum number of guest: " + QString::number(pChannel->getMaxGuestsNum()) + "\n" +
                              pChannel->getDescription());
    }
    else {
        if (ui->cmdEdit->text() == "&Edit")
            ui->cmdEdit->setEnabled(false);                                       // make cmd Edit disabled
        ui->cmdChoose->setEnabled(false);                                         // make cmd Choose disabled
        pairChoosenGuestChannel = qMakePair(nullptr, nullptr);
        ui->textEdit->setText("Good Vibes Radio\n" + QDateTime::currentDateTime().toString("ddd, d MMMM HH:mm"));
    }
}

ChannelWidget* ChooseChannelWidget::getHostChannel(const QString& channelName) {        // find host channel widget by name
    for (auto it = hostChannelsList.begin(); it != hostChannelsList.end(); it++) {
        if ((*it)->getChannel()->getChannelName() == channelName)
            return *it;
    }
    return nullptr;
}

Channel* ChooseChannelWidget::getGuestChannel(const QString &channelName) {             // find guest channel by name
    for (auto it = guestChannelsList.begin(); it != guestChannelsList.end(); it++) {
        if ((*it)->getChannelName() == channelName)
            return *it;
    }
    return nullptr;
}


void ChooseChannelWidget::slotChooseClicked() {                                         // handle cmd choose click
    if (pairChoosenHostChannel.first != nullptr && pairChoosenHostChannel.second != nullptr) {
        if (pMainWindow != nullptr)
            pMainWindow->setWidget(pairChoosenHostChannel.first);
        (pairChoosenHostChannel.first)->unmute();
    }
    else if (pairChoosenGuestChannel.first != nullptr && pairChoosenGuestChannel.second != nullptr) {
        if (pPlayerWidget->isEnabled()) {
            bool access = true;
            if (pairChoosenGuestChannel.first->getPrivateStatus()) {
                if (pCheckingPasswdDialog->exec()) {
                    if (pCheckingPasswdDialog->getPasswdLine() != pairChoosenGuestChannel.first->getPassword())
                        access = false;
                }
            }

            if (access) {
                pPlayerWidget->setChannel(pairChoosenGuestChannel.first);
                if (pMainWindow != nullptr)
                    pMainWindow->setWidget(pPlayerWidget);
            }
            else {
                ui->textEdit->append("Incorrect password.");
            }
        }
        else {
            ui->textEdit->append("This channel is not enabled yet.");
        }
    }
}

void ChooseChannelWidget::slotTurnOffChannel() {                                        // handle cmd turn off click
    if (pMainWindow != nullptr)
        pMainWindow->removeWidget(pairChoosenHostChannel.first);
    backToChooseChannel();
    hostChannelsList.removeAll(pairChoosenHostChannel.first);
    hostButtons.removeAll(pairChoosenHostChannel.second);
    if (hostChannelsList.isEmpty()) {
        for (auto it = guestButtons.begin(); it != guestButtons.end(); it++)
            (*it)->setEnabled(true);
    }
    ui->channelsLayout->removeWidget(pairChoosenHostChannel.second);
    if (ui->textEdit->toPlainText().contains("Maximum number of guest: "))
        ui->textEdit->setText("Good Vibes Radio\n" + QDateTime::currentDateTime().toString("ddd, d MMMM HH:mm"));
    if (ui->cmdEdit->text() == "&Edit")
        ui->cmdEdit->setEnabled(false);
    ui->cmdChoose->setEnabled(false);
    delete (pairChoosenHostChannel.first)->getChannel();
    delete pairChoosenHostChannel.first;
    delete pairChoosenHostChannel.second;
    pairChoosenHostChannel = qMakePair(nullptr, nullptr);
}

void ChooseChannelWidget::slotDataReady(QByteArray data) {                  // handle recieving the data from server
    qDebug() << "read channls info";
    if (pairChoosenGuestChannel.second != nullptr) {
        pairChoosenGuestChannel = qMakePair(nullptr, nullptr);
        if (ui->textEdit->toPlainText().contains("Maximum number of guest: "))
            ui->textEdit->setText("Good Vibes Radio\n" + QDateTime::currentDateTime().toString("ddd, d MMMM HH:mm"));
    }
    if (!guestChannelsList.isEmpty()) {
        guestChannelsList.erase(guestChannelsList.begin(), guestChannelsList.end());
    }
    if (!guestButtons.isEmpty()) {
        for (auto it = guestButtons.begin(); it != guestButtons.end(); it++) {
            ui->channelsLayout->removeWidget(*it);
            QCommandLinkButton* pButton = *it;
            guestButtons.removeAll(*it);
            delete pButton;
        }
    }
    QList<Channel> channelList;
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> channelList;
    if (!channelList.isEmpty()) {
        pPlayerWidget = new PlayerWidget;
        if (pMainWindow != nullptr)
            pMainWindow->addWidget(pPlayerWidget);
    }
    else if (pPlayerWidget != nullptr) {
        if (pMainWindow != nullptr)
            pMainWindow->removeWidget(pPlayerWidget);
        delete pPlayerWidget;
        pPlayerWidget = nullptr;
    }
    for (auto it = channelList.begin(); it != channelList.end(); it++) {
        guestChannelsList.append(new Channel(*it));
        QCommandLinkButton* pChannelButton = new QCommandLinkButton((*it).getChannelName());
        pChannelButton->setDescription((*it).getHostName());
        pChannelButton->setCheckable(true);
        pChannelButton->setAutoExclusive(true);
        pChannelButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        int row, col, rs, cs;
        ui->channelsLayout->getItemPosition((ui->channelsLayout)->indexOf(ui->guestLine), &row, &col, &rs, &cs);
        ui->channelsLayout->removeWidget(ui->hostLine);
        ui->channelsLayout->addWidget(pChannelButton, row, col, rs, cs);
        ui->channelsLayout->addWidget(ui->guestLine, row + 1, col, rs, cs);
        connect(pChannelButton, SIGNAL(toggled(bool)),
                this, SLOT(slotGuestChannelClicked()));
        guestButtons.append(pChannelButton);
    }
}

void ChooseChannelWidget::slotReconnect() {
    emit connectToServer();
}

void ChooseChannelWidget::slotConnected() {
    disconnect(ui->cmdEdit, SIGNAL(clicked()),
            this, SLOT(slotReconnect()));
    connect(ui->cmdEdit, SIGNAL(clicked()),
             this, SLOT(slotEditChannel()));

    ui->cmdEdit->setText("&Edit");
    if (pairChoosenHostChannel.second == nullptr)
        ui->cmdEdit->setEnabled(false);
    ui->textEdit->setText("Good Vibes Radio\n" + QDateTime::currentDateTime().toString("ddd, d MMMM HH:mm"));
    ui->cmdCreateChannel->setEnabled(true);
    for (auto it = guestButtons.begin(); it != guestButtons.end(); it++)
        (*it)->setEnabled(true);
    for (auto it = hostButtons.begin(); it != hostButtons.end(); it++)
        (*it)->setEnabled(true);
}

void ChooseChannelWidget::slotDisconnected() {
    ui->cmdEdit->setText("&Reconnect");
    disconnect(ui->cmdEdit, SIGNAL(clicked()),
             this, SLOT(slotEditChannel()));
    connect(ui->cmdEdit, SIGNAL(clicked()),
            this, SLOT(slotReconnect()));
    ui->cmdEdit->setEnabled(true);
}

void ChooseChannelWidget::slotConnectionError(QString strError) {
    emit disconnectFromServer();
    ui->cmdCreateChannel->setEnabled(false);
    ui->textEdit->setText(strError);
    for (auto it = guestButtons.begin(); it != guestButtons.end(); it++)
        (*it)->setEnabled(false);

    if (pairChoosenGuestChannel.first != nullptr && pairChoosenGuestChannel.second != nullptr) {
        pairChoosenGuestChannel.second->setChecked(false);
        pairChoosenGuestChannel = qMakePair(nullptr, nullptr);
    }
}

void ChooseChannelWidget::backToChooseChannel() {                       // handle cmd back click
    emit sendString("<request>");
    if (pMainWindow != nullptr)
        pMainWindow->setWidget(this);
}

