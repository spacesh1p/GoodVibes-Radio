#include <QTcpSocket>
#include <QStringList>
#include <QTime>
#include "playerwidget.h"
#include "ui_playerwidget.h"
#include "channel.h"
#include "choosechannelwidget.h"
#include "socketthread.h"

PlayerWidget::PlayerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerWidget)
{
    ui->setupUi(this);
    QString parentName = parent->metaObject()->className();
    if (parentName == "ChooseChannelWidget")
        pChooseChannelWidget = (ChooseChannelWidget*)parent;
    else
        pChooseChannelWidget = nullptr;

    if (pChooseChannelWidget != nullptr)
        userName = pChooseChannelWidget->getUserName();

    pMediaPlayer = new QMediaPlayer(this);
    connect(pMediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)));
    connect(ui->volSlider, SIGNAL(valueChanged(int)),
            pMediaPlayer, SLOT(setVolume(int)));
    connect(ui->volSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotVolumeChanged(int)));
    connect(ui->volIcon, SIGNAL(clicked(bool)),
            pMediaPlayer, SLOT(setMuted(bool)));
    connect(ui->volIcon, SIGNAL(clicked(bool)),
            this, SLOT(slotVolumeClicked(bool)));
    pReaderThread = new SocketThread();
    connect(pReaderThread, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotDataReady(QByteArray)));
    connect(pReaderThread, SIGNAL(connectionError(QString)),
            this, SLOT(slotError(QString)));
    connect(pReaderThread, SIGNAL(connectedToServer()),
            this, SLOT(slotConnected()));
    connect(pReaderThread, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    connect(this, SIGNAL(connectToServer()),
            pReaderThread, SLOT(slotConnectToServer()));
    connect(this, SIGNAL(disconnectFromServer()),
            pReaderThread, SLOT(slotDisconnectFromServer()));
    connect(this, SIGNAL(sendString(QString)),
            pReaderThread, SLOT(slotSendString(QString)));

    ui->volSlider->setValue(50);
    connect(ui->cmdBack, SIGNAL(clicked()),
            this, SLOT(slotBackClicked()));

    ui->cmdSend->setEnabled(false);
    connect(ui->cmdSend, SIGNAL(clicked()),
            this, SLOT(slotSendMessage()));
    enabled = true;
}

void PlayerWidget::setChannel(Channel* channel) {
    connect(pReaderThread, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    pChannel = channel;
    ui->channelName->setText(pChannel->getChannelName());
    ui->textEdit->setText("");
    ui->volSlider->setValue(50);
    pReaderThread->setDescription("<user:" + userName + ">,<host:" + pChannel->getHostName() + ">,<name:" + pChannel->getChannelName() + ">,<purpose:readMedia>");
    emit connectToServer();
}

PlayerWidget::~PlayerWidget()
{
    pReaderThread->deleteLater();
    delete pMediaPlayer;
    delete ui;
}

bool PlayerWidget::isEnabled() {
    return enabled;
}

void PlayerWidget::slotMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::MediaStatus::LoadedMedia) {
        pMediaPlayer->play();
        pMediaPlayer->setVolume(ui->volSlider->value());
    }
    if (status == QMediaPlayer::MediaStatus::EndOfMedia)
        setNextSong();
}

void PlayerWidget::slotSetPosition(bool seekable) {
    if (seekable && buffer.isOpen()) {
        quint64 position = positionQueue.dequeue();
        pMediaPlayer->setPosition(position);
        disconnect(pMediaPlayer, SIGNAL(seekableChanged(bool)),
                   this, SLOT(slotSetPosition(bool)));
    }
}

void PlayerWidget::slotConnected() {
    ui->cmdSend->setEnabled(true);
    firstSong = true;
    enabled = false;
    ui->textEdit->append(pChannel->getWelcome());
    ui->cmdBack->setText("&Disconnect");
    disconnect(ui->cmdBack, SIGNAL(clicked()),
            this, SLOT(slotBackClicked()));
    connect(ui->cmdBack, SIGNAL(clicked()),
            this, SLOT(slotDisconnectFromChannel()));
}

void PlayerWidget::slotDisconnected() {
    ui->cmdSend->setEnabled(false);
    enabled = true;
}

void PlayerWidget::slotError(const QString &strError) {
    ui->cmdSend->setEnabled(false);
    if (buffer.isOpen())
        buffer.close();
    ui->songNameLine->setText("");
    ui->textEdit->append(strError);
    ui->cmdBack->setText("&Back");
    disconnect(ui->cmdBack, SIGNAL(clicked()),
            this, SLOT(slotDisconnectFromChannel()));
    connect(ui->cmdBack, SIGNAL(clicked()),
            this, SLOT(slotBackClicked()));
}


void PlayerWidget::slotDataReady(QByteArray data) {
    QString description;
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> description;
    QStringList descriptList = description.split(",", QString::SkipEmptyParts);
    QStringList identifier;
    if (!descriptList.isEmpty())
        identifier = descriptList[0].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts);
    if (!identifier.isEmpty()) {
        if (identifier[0] == "guests") {
            ui->lcdNumOfGuests->display(identifier[1]);
            QStringList msg = descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts);
            if (msg[0] != userName) {
                ui->textEdit->append(msg[0] + " " + msg[1]);
            }
        }
        else if (identifier[0] == "song") {
            QString songName = (descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
            QByteArray* arr = new QByteArray;
            QString timeStr;
            in >> (*arr) >> timeStr;
            QString pos = (timeStr.split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
            quint64 position;
            if (pos == "0")
                position = quint64(0);
            else
                position = quint64(QTime::fromString(pos, "hh.mm.ss.zzz").msecsTo(QTime::currentTime()));
            songsQueue.enqueue(qMakePair(songName, arr));
            positionQueue.enqueue(position);
            if (firstSong) {
                firstSong = false;
                setNextSong();
            }
        }
        else if (identifier[0] == "skip") {
            setNextSong();
        }
        else if (identifier[0] == "msg") {
            QStringList senderInfo = (descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts));
            QString senderName = senderInfo[1];
            if (senderInfo[0] == "host")
                senderName += "[host]";
            QStringList timeList = (descriptList[2].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts));
            ui->textEdit->append(timeList[1] + ":" + timeList[2] + ":" + timeList[3] +
                                 " " + senderName + ": " + identifier[1]);
        }
    }
}

void PlayerWidget::setNextSong() {
    if (!songsQueue.isEmpty()) {
        QPair<QString, QByteArray*> songPair = songsQueue.dequeue();
        ui->songNameLine->setText(songPair.first);
        if (buffer.isOpen())
            buffer.close();
        buffer.setBuffer(songPair.second);
        buffer.open(QIODevice::ReadOnly);
        pMediaPlayer->setMedia(QMediaContent(), &buffer);
        connect(pMediaPlayer, SIGNAL(seekableChanged(bool)),
                this, SLOT(slotSetPosition(bool)));
    }
    else {
        ui->songNameLine->setText("");
        if (buffer.isOpen())
            buffer.close();
    }
}

void PlayerWidget::slotBackClicked() {
    ui->songNameLine->setText("");
    songsQueue.clear();
    positionQueue.clear();
    if (buffer.isOpen())
        buffer.close();
    pChooseChannelWidget->backToChooseChannel();
}

void PlayerWidget::slotDisconnectFromChannel() {
    ui->songNameLine->setText("");
    songsQueue.clear();
    positionQueue.clear();
    if (buffer.isOpen())
        buffer.close();
    emit disconnectFromServer();
    pChooseChannelWidget->backToChooseChannel();
}

void PlayerWidget::slotVolumeClicked(bool state) {
    if (state)
        ui->volIcon->setIcon(QIcon(":/new/prefix1/icons/mute-volume.png"));
    else if (ui->volSlider->value() <= 50)
        ui->volIcon->setIcon(QIcon(":/new/prefix1/icons/low-volume.png"));
    else
        ui->volIcon->setIcon(QIcon(":/new/prefix1/icons/high-volume.png"));
}

void PlayerWidget::slotVolumeChanged(int val) {
    if (val <= 50)
        ui->volIcon->setIcon(QIcon(":/new/prefix1/icons/low-volume.png"));
    else
        ui->volIcon->setIcon(QIcon(":/new/prefix1/icons/high-volume.png"));
}

void PlayerWidget::slotSendMessage() {
    emit sendString("<msg:" + ui->msgEdit->text() + ">,<guest:" + userName + ">");
    ui->msgEdit->setText("");
}
