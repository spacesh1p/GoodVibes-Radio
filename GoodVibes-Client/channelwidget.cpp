#include <QFileDialog>
#include <QDataStream>
#include <QtWidgets>
#include "channelwidget.h"
#include "ui_channelwidget.h"
#include "channelsettingsdialog.h"
#include "choosechannelwidget.h"
#include "channel.h"
#include "mediahandler.h"
#include "socketthread.h"

ChannelWidget::ChannelWidget(Channel* channel, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChannelWidget)
    , pChannel(channel)
{
    ui->setupUi(this);                                  // set up GUI

    QString parentName = parent->metaObject()->className();
    if (parentName == "ChooseChannelWidget")                            // if parent is ChooseChannelWidget
        pChooseChannelWidget = (ChooseChannelWidget*)parent;
    else
        pChooseChannelWidget = nullptr;

    if (pChooseChannelWidget != nullptr)
        userName = pChooseChannelWidget->getUserName();

    pSettingsDialog = new ChannelSettingsDialog(pChannel, this);        // create setting dialog object
    connect(pChooseChannelWidget, SIGNAL(channelCreated()),
            this, SLOT(slotChannelCreated()));

    pMediaHandler = nullptr;
    pTextSender = nullptr;
    pMediaSender = nullptr;
    callCnt = 0;
}

ChannelWidget::~ChannelWidget()
{
    delete ui;
    delete pSettingsDialog;
    if (pChannel != nullptr)
        delete pChannel;
    if (pMediaHandler != nullptr)
        delete pMediaHandler;
    if (pTextSender != nullptr)
        pTextSender->deleteLater();
    if (pMediaSender != nullptr)
        pMediaSender->deleteLater();
}

int ChannelWidget::openSettingsDialog() {
    return pSettingsDialog->exec();                        // open dialog window
}

Channel* ChannelWidget::getChannel() {
    return pChannel;
}

void ChannelWidget::unmute() {
    pMediaPlayer->setMuted(ui->cmdVolume->isChecked());
}


void ChannelWidget::addSongButton(QCommandLinkButton* button) {
    ui->scrollLayout->addWidget(button);
}

void ChannelWidget::removeSongButton(QCommandLinkButton* button) {
    ui->scrollLayout->removeWidget(button);
}

SocketThread* ChannelWidget::getMediaSender() {
    return pMediaSender;
}

int ChannelWidget::getSliderValue() {
    return ui->sldVolume->value();
}

void ChannelWidget::slotOpenFiles() {
    QStringList filesList = QFileDialog::getOpenFileNames(this, "Open files", "", "*.mp3");       // open the dialog window for choosing songs *.mp3, *.wav
    for (QStringList::iterator it = filesList.begin(); it != filesList.end(); it++) {
        pMediaHandler->addSong(*it);
    }
}

void ChannelWidget::slotConnected() {
    if (!callCnt)
        ui->textView->append("Connection received.");
    if (pTextSender->isConnected() && pMediaSender->isConnected()) {
        ui->cmdAddSong->setEnabled(true);
        ui->cmdSend->setEnabled(true);
        disconnect(ui->cmdBack, SIGNAL(clicked()),
                this, SLOT(slotRestart()));
        connect(ui->cmdBack, SIGNAL(clicked()),                                 // handle cmd Back clicked
                this, SLOT(slotBackClicked()));
        ui->cmdBack->setText("&Back");
        ui->cmdBack->setEnabled(true);
    }

    callCnt++;
    if (callCnt == 2)
        callCnt = 0;
}

void ChannelWidget::slotError(const QString& strError) {
    QString str = ui->textView->toPlainText();
    if (!str.contains(strError))
        ui->textView->append(strError);
    ui->cmdAddSong->setEnabled(false);
    ui->cmdSend->setEnabled(false);
    ui->cmdBack->setEnabled(false);
    if (pMediaSender->isConnected() || pTextSender->isConnected())
        emit disconnectFromServer();
    else {
        ui->cmdBack->setEnabled(true);
    }
}

void ChannelWidget::slotDisconnected() {
    if (!callCnt)
        ui->textView->append("Disconnected from server.");

    if (!pTextSender->isConnected() && !pMediaSender->isConnected()) {
        ui->cmdAddSong->setEnabled(false);
        ui->cmdSend->setEnabled(false);
        ui->cmdBack->setText("&Reconnect");
        disconnect(ui->cmdBack, SIGNAL(clicked()),
                this, SLOT(slotBackClicked()));
        connect(ui->cmdBack, SIGNAL(clicked()),
                this, SLOT(slotRestart()));
        ui->cmdBack->setEnabled(true);
    }
    callCnt++;
    if (callCnt == 2)
        callCnt = 0;
}

void ChannelWidget::slotDataReady(QByteArray data) {
    QString description;
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> description;
    QStringList descriptList = description.split(",");
    QStringList identifier;
    if (!descriptList.isEmpty())
        identifier = descriptList[0].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts);
    if (!identifier.isEmpty()) {
        if (identifier[0] == "guests") {
            ui->lcdNumber->display(identifier[1]);
            QStringList msg = descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts);
            ui->textView->append(msg[0] + " " + msg[1]);
        }
        else if (identifier[0] == "request") {
            slotUpdateData();
        }
        else if (identifier[0] == "msg") {
            QStringList senderInfo = (descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts));
            QString senderName = senderInfo[1];
            if (senderInfo[0] == "host")
                senderName += "[host]";
            QStringList timeList = (descriptList[2].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts));
            ui->textView->append(timeList[1] + ":" + timeList[2] + ":" + timeList[3] +
                                 " " + senderName + ": " + identifier[1]);
        }
    }
}

void ChannelWidget::slotRestart() {
    emit connectToServer();
}

void ChannelWidget::slotBackClicked() {
    pMediaPlayer->setMuted(true);
    if (pChooseChannelWidget != nullptr)
        pChooseChannelWidget->backToChooseChannel();
}

void ChannelWidget::slotFileNotOpened() {
    ui->textView->append("Cannot open file.");
}

void ChannelWidget::slotChannelCreated() {
    disconnect(pChooseChannelWidget, SIGNAL(channelCreated()),
            this, SLOT(slotChannelCreated()));
    connect(pSettingsDialog, SIGNAL(settingsSeted()),
            this, SLOT(slotUpdateData()));
    pTextSender = new SocketThread();                                                                   // create TextSender
    pTextSender->setDescription(QString("<host:" + userName + ">,<name:" + pChannel->getChannelName() + ">,<purpose:sendChannelInfo>"));
    pMediaSender = new SocketThread();                                                                  // create MediaSender
    pMediaSender->setDescription(QString("<host:" + userName + ">,<name:" + pChannel->getChannelName() + ">,<purpose:sendMedia>"));

    connect(pTextSender, SIGNAL(connectedToServer()),
            this, SLOT(slotConnected()));
    connect(pTextSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    connect(pTextSender, SIGNAL(connectionError(QString)),
            this, SLOT(slotError(QString)));
    connect(pTextSender, SIGNAL(fileNotOpened()),
            this, SLOT(slotFileNotOpened()));
    connect(pTextSender, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotDataReady(QByteArray)));
    connect(this, SIGNAL(connectToServer()),
            pTextSender, SLOT(slotConnectToServer()));
    connect(this, SIGNAL(disconnectFromServer()),
            pTextSender, SLOT(slotDisconnectFromServer()));
    connect(this, SIGNAL(sendChannelData(Channel)),
            pTextSender, SLOT(slotSendChannelData(Channel)));
    connect(pMediaSender, SIGNAL(connectedToServer()),
            this, SLOT(slotConnected()));
    connect(pMediaSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    connect(pMediaSender, SIGNAL(connectionError(QString)),
            this, SLOT(slotError(QString)));
    connect(pMediaSender, SIGNAL(fileNotOpened()),
            this, SLOT(slotFileNotOpened()));
    connect(this, SIGNAL(connectToServer()),
            pMediaSender, SLOT(slotConnectToServer()));
    connect(this, SIGNAL(disconnectFromServer()),
            pMediaSender, SLOT(slotDisconnectFromServer()));
    connect(this, SIGNAL(sendString(QString)),
            pMediaSender, SLOT(slotSendString(QString)));

    emit connectToServer();

    pMediaHandler = new MediaHandler(this);                                 // create MediaHandler object
    pMediaPlayer = pMediaHandler->getMediaPlayer();

    connect(pMediaPlayer, SIGNAL(seekableChanged(bool)),
            this, SLOT(slotSeekable(bool)));

    ui->cmdAddSong->setEnabled(false);
    ui->cmdBack->setEnabled(false);
    ui->cmdSkip->setEnabled(false);
    ui->cmdSend->setEnabled(false);
    ui->sldVolume->setValue(50);

    connect(ui->cmdSend, SIGNAL(clicked()),
            this, SLOT(slotSendMessage()));
    connect(ui->msgEdit, SIGNAL(returnPressed()),
            this, SLOT(slotSendMessage()));
    connect(ui->cmdSkip, SIGNAL(clicked()),
            this, SLOT(slotSkip()));
    connect(ui->cmdAddSong, SIGNAL(clicked()),                              // handle Add Song button clicked
            this, SLOT(slotOpenFiles()));
    connect(ui->cmdTurnOff, SIGNAL(clicked()),
            pChooseChannelWidget, SLOT(slotTurnOffChannel()));
    connect(ui->cmdVolume, SIGNAL(clicked(bool)),
            pMediaPlayer, SLOT(setMuted(bool)));
    connect(ui->cmdVolume, SIGNAL(clicked(bool)),
            this, SLOT(slotVolumeClicked(bool)));
    connect(ui->sldVolume, SIGNAL(valueChanged(int)),
            pMediaPlayer, SLOT(setVolume(int)));
    connect(ui->sldVolume, SIGNAL(valueChanged(int)),
            this, SLOT(slotVolumeChanged(int)));
    connect(pMediaSender, SIGNAL(connectionError(QString)),
            pMediaHandler, SLOT(slotError()));

    ui->channelName->setText(pChannel->getChannelName());
    ui->lcdNumber->display(0);
}

void ChannelWidget::slotUpdateData() {
    emit sendChannelData(*pChannel);
}

void ChannelWidget::slotVolumeClicked(bool state) {
    if (state)
        ui->cmdVolume->setIcon(QIcon(":/new/prefix1/icons/mute-volume.png"));
    else if (ui->sldVolume->value() <= 50)
        ui->cmdVolume->setIcon(QIcon(":/new/prefix1/icons/low-volume.png"));
    else
        ui->cmdVolume->setIcon(QIcon(":/new/prefix1/icons/high-volume.png"));
}

void ChannelWidget::slotVolumeChanged(int val) {
    if (val <= 50)
        ui->cmdVolume->setIcon(QIcon(":/new/prefix1/icons/low-volume.png"));
    else
        ui->cmdVolume->setIcon(QIcon(":/new/prefix1/icons/high-volume.png"));
}

void ChannelWidget::slotSeekable(bool state) {
    if (state)
        ui->cmdSkip->setEnabled(true);
    else
        ui->cmdSkip->setEnabled(false);

}

void ChannelWidget::slotSkip() {
    emit sendString("<skip>");
    pMediaPlayer->setPosition(pMediaPlayer->duration());
}

void ChannelWidget::slotSendMessage() {
    if (!ui->msgEdit->text().isEmpty()) {
        emit sendString("<msg:" + ui->msgEdit->text() + ">,<host:" + userName + ">");
        ui->msgEdit->setText("");
    }
}

