#include <QStringList>
#include <QtAlgorithms>
#include <QCommandLinkButton>
#include "mediahandler.h"
#include "socketthread.h"
#include "channelwidget.h"

MediaHandler::MediaHandler(ChannelWidget* channelWidget)
    : pChannelWidget(channelWidget)
{
    pMediaPlayer = new QMediaPlayer;
    pMediaSender = pChannelWidget->getMediaSender();
    connect(this, SIGNAL(readyToSendFile(QString)),
            pMediaSender, SLOT(slotSendFileData(QString)));
    connect(pMediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)));
    connect(this, SIGNAL(songAdded(int)),
            this, SLOT(slotSongAdded(int)));
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
    if (!contains(path)) {
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
    }
    else
        songName = "";
    return songName;
}

void MediaHandler::addSong(const QString& path) {
    QString songName = getFileName(path);
    if (songName != "") {
        QCommandLinkButton* pButton = new QCommandLinkButton(songName);             // create buttons for songs
        pButton->setCheckable(true);
        pButton->setAutoExclusive(true);
        pChannelWidget->addSongButton(pButton);
        playList.enqueue(qMakePair(path, pButton));
        filesQueue.enqueue(path);
        emit songAdded(playList.size());
    }
}

void MediaHandler::setMuted(bool state) {
    pMediaPlayer->setMuted(state);
}

void MediaHandler::slotSongAdded(int number) {
    if (number <= 3) {
        emit readyToSendFile(filesQueue.dequeue());
        if (number == 1) {
            pMediaPlayer->setMedia(QUrl::fromLocalFile(playList.head().first));
            playList.head().second->setChecked(true);
        }
    }
    else {
        disconnect(this, SIGNAL(songAdded(int)),
                   this, SLOT(slotSongAdded(int)));
        connect(pMediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                this, SLOT(slotSendNextSong(QMediaPlayer::MediaStatus)));
    }
}

void MediaHandler::slotMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::MediaStatus::LoadedMedia) {
        pMediaPlayer->play();
        pMediaPlayer->setVolume(pChannelWidget->getSliderVlaue());
    }

    if (status == QMediaPlayer::MediaStatus::EndOfMedia) {
        pChannelWidget->removeSongButton(playList.head().second);
        delete playList.head().second;
        playList.dequeue();
        if (!playList.isEmpty()) {
            pMediaPlayer->setMedia(QUrl::fromLocalFile(playList.head().first));
            playList.head().second->setChecked(true);
        }
        else
            connect(this, SIGNAL(songAdded(int)),
                    this, SLOT(slotSongAdded(int)));
    }
}

void MediaHandler::slotSendNextSong(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::MediaStatus::EndOfMedia)
        if (!filesQueue.isEmpty())
            emit readyToSendFile(filesQueue.dequeue());
}

void MediaHandler::slotMutedChanged(bool state) {
    pMediaPlayer->setMuted(state);
}


void MediaHandler::slotChangeVolume(int value) {
    pMediaPlayer->setVolume(value);
}