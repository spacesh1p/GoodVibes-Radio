#include <QStringList>
#include <QtAlgorithms>
#include <QCommandLinkButton>
#include <QTime>
#include <QToolButton>
#include <QtDebug>
#include "mediahandler.h"
#include "socketthread.h"
#include "channelwidget.h"

MediaHandler::MediaHandler(ChannelWidget* channelWidget)
    : pChannelWidget(channelWidget)
{
    pMediaPlayer = new QMediaPlayer;
    pMediaSender = pChannelWidget->getMediaSender();
    connect(this, SIGNAL(readyToSendFile(QString, QString)),
            pMediaSender, SLOT(slotSendFileData(QString, QString)));
    connect(this, SIGNAL(sendString(QString)),
            pMediaSender, SLOT(slotSendString(QString)));
    connect(pMediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)));
    connect(this, SIGNAL(songAdded(int)),
            this, SLOT(slotSongAdded(int)));
    isConnected = true;
}

MediaHandler::~MediaHandler() {
    delete pMediaPlayer;
}

bool MediaHandler::contains(const QString& path) {
    for (auto it = playList.begin(); it != playList.end(); it++)
        if ((*it).first == path)
            return true;
    return false;
}

QString MediaHandler::getFileName(const QString& path) {
    QString songName;
    bool write = false;
    for (int i = path.length() - 2; i != 0; i--) {              // method of getting song name
        if (path[i] == '.') {
            write = true;
            continue;
        }
        if (path[i] == '/')
            break;
        if (write)
            songName.push_back(path[i]);
    }
    for (int i = 0; i < songName.length() / 2; i++) {
        QChar c = songName[i];
        songName[i] = songName[songName.length() - 1 - i];
        songName[songName.length() - 1 - i] = c;
    }
    return songName;
}

void MediaHandler::addSong(const QString& path) {
    if (!contains(path)) {
        QString songName = getFileName(path);
        QCommandLinkButton* pButton = new QCommandLinkButton(songName);             // create buttons for songs
        pButton->setCheckable(true);
        pButton->setAutoExclusive(true);
        pButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        pButton->setIcon(QIcon(":/new/prefix1/icons/interface.png"));
        pChannelWidget->addSongButton(pButton);
        playList.enqueue(qMakePair(path, pButton));
        filesQueue.enqueue(path);
        emit songAdded(playList.size());
    }
}

void MediaHandler::slotSongAdded(int number) {
    if (number <= 3) {
        QString path = filesQueue.dequeue();
        emit readyToSendFile(getFileName(path), path);
        if (number == 1) {
            pMediaPlayer->setMedia(QUrl::fromLocalFile(playList.head().first));
            playList.head().second->setChecked(true);
        }
    }
    else {
        disconnect(this, SIGNAL(songAdded(int)),
                   this, SLOT(slotSongAdded(int)));
        isConnected = false;
        connect(pMediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                this, SLOT(slotSendNextSong(QMediaPlayer::MediaStatus)));
    }
}

void MediaHandler::slotMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::MediaStatus::LoadedMedia) {
        pMediaPlayer->play();
        emit sendString("<startMedia:" + QTime::currentTime().toString("hh.mm.ss.zzz") + ">");
        pMediaPlayer->setVolume(pChannelWidget->getSliderValue());
    }

    if (status == QMediaPlayer::MediaStatus::EndOfMedia) {
        emit sendString("<endOfMedia>");
        pChannelWidget->removeSongButton(playList.head().second);
        delete playList.head().second;
        playList.dequeue();
        if (!playList.isEmpty()) {
            pMediaPlayer->setMedia(QUrl::fromLocalFile(playList.head().first));
            playList.head().second->setChecked(true);
        }
        else if (!isConnected) {
            disconnect(pMediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                       this, SLOT(slotSendNextSong(QMediaPlayer::MediaStatus)));
            connect(this, SIGNAL(songAdded(int)),
                    this, SLOT(slotSongAdded(int)));
        }
    }
}

void MediaHandler::slotSendNextSong(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::MediaStatus::EndOfMedia)
        if (!filesQueue.isEmpty()) {
            QString path = filesQueue.dequeue();
            emit readyToSendFile(getFileName(path), path);
        }
}

void MediaHandler::slotError() {
    pMediaPlayer->setMedia(QMediaContent());
    filesQueue.clear();
    for (auto it = playList.begin(); it != playList.end(); ++it) {
        pChannelWidget->removeSongButton(it->second);
        delete it->second;
    }
    playList.clear();
}

QMediaPlayer* MediaHandler::getMediaPlayer() {
    return pMediaPlayer;
}
