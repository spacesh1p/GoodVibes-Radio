#include <QTcpSocket>
#include <QStringList>
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

    pMediaPlayer = new QMediaPlayer(this);
    connect(pMediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)));
    connect(ui->volSlider, SIGNAL(valueChanged(int)),
            pMediaPlayer, SLOT(setVolume(int)));
    connect(ui->volIcon, SIGNAL(clicked(bool)),
            pMediaPlayer, SLOT(setMuted(bool)));
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
    emit connectToServer();

    ui->volSlider->setValue(50);
    connect(ui->cmdBack, SIGNAL(clicked()),
            this, SLOT(slotBackClicked()));
    enabled = true;
}

void PlayerWidget::setChannel(Channel* channel) {
    connect(pReaderThread, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotRestart()));
    pChannel = channel;
    ui->channelName->setText(pChannel->getName());
    ui->textEdit->setText("");
    ui->volSlider->setValue(50);
    pReaderThread->setDescription("<user:name>,<name:" + pChannel->getName() + ">,<purpose:readMedia>");
    buffer.open(QIODevice::ReadOnly);
    emit connectToServer();
}

PlayerWidget::~PlayerWidget()
{
    delete pReaderThread;
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
    if (seekable) {
        pMediaPlayer->setPosition(position);
        position = 0;
        disconnect(pMediaPlayer, SIGNAL(seekableChanged(bool)),
                   this, SLOT(slotSetPosition(bool)));
    }
}

void PlayerWidget::slotConnected() {
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
    enabled = true;
}

void PlayerWidget::slotError(const QString &strError) {
    ui->textEdit->append(strError);
    ui->cmdBack->setText("&Back");
    disconnect(ui->cmdBack, SIGNAL(clicked()),
            this, SLOT(slotDisconnectFromChannel()));
    connect(ui->cmdBack, SIGNAL(clicked()),
            this, SLOT(slotBackClicked()));
}

void PlayerWidget::slotRestart() {
    ui->textEdit->append("Disconnected.");
    emit connectToServer();
}

void PlayerWidget::slotDataReady(QByteArray data) {
    QString description;
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> description;
    QStringList descriptList = description.split(",");
    QStringList identifier;
    if (!descriptList.isEmpty())
        identifier = descriptList[0].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts);
    if (!identifier.isEmpty()) {
        if (identifier[0] == "guests")
            ui->lcdNumOfGuests->display(identifier[1]);
        if (identifier[0] == "song") {
            QString songName = (descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
            QByteArray* arr = new QByteArray;
            in >> (*arr) >> position;
            songsQueue.enqueue(qMakePair(songName, arr));
            if (firstSong) {
                firstSong = false;
                connect(pMediaPlayer, SIGNAL(seekableChanged(bool)),
                        this, SLOT(slotSetPosition(bool)));
                setNextSong();
            }
        }
    }
}

void PlayerWidget::setNextSong() {
    if (!songsQueue.isEmpty()) {
        QPair<QString, QByteArray*> songPair = songsQueue.dequeue();
        ui->songNameLine->setText(songPair.first);
        buffer.setBuffer(songPair.second);
        pMediaPlayer->setMedia(QMediaContent(), &buffer);
    }
}

void PlayerWidget::slotBackClicked() {
    pMediaPlayer->stop();
    songsQueue.clear();
    buffer.close();
    disconnect(pReaderThread, SIGNAL(disconnectedFromServer()),
               this, SLOT(slotRestart()));
    connect(pReaderThread, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    pChooseChannelWidget->backToChooseChannel();
}

void PlayerWidget::slotDisconnectFromChannel() {
    pMediaPlayer->stop();
    songsQueue.clear();
    buffer.close();
    disconnect(pReaderThread, SIGNAL(disconnectedFromServer()),
               this, SLOT(slotRestart()));
    connect(pReaderThread, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    emit disconnectFromServer();
    pChooseChannelWidget->backToChooseChannel();
}
