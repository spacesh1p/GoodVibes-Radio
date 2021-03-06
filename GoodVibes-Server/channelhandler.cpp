#include <QDataStream>
#include <QTime>
#include <socketthread.h>
#include "channelhandler.h"
#include "server.h"

ChannelHandler::ChannelHandler(QObject *parent)
    : QObject(parent)
{

    QString parentClass = parent->metaObject()->className();
    if(parentClass == "Server")
        pServer = (Server*)parent;
    else
        pServer = nullptr;

    pHostTextSender = nullptr;
    pHostMediaSender = nullptr;
    disconnected = false;
}

ChannelHandler::~ChannelHandler() {
    if (pHostTextSender != nullptr)
        pHostTextSender->deleteLater();
    if (pHostMediaSender != nullptr)
        pHostMediaSender->deleteLater();
    for (auto it = usersMediaSockets.begin(); it != usersMediaSockets.end(); it++)
        it.value()->deleteLater();
}

void ChannelHandler::setHostTextSocket(SocketThread* socketThread) {
    pHostTextSender = socketThread;
    connect(pHostTextSender, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotTextDataReady(QByteArray)));
    connect(pHostTextSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconected()));
    connect(this, SIGNAL(sendMessage(QString)),
            pHostTextSender, SLOT(slotSendString(QString)));
    connect (this, SIGNAL(sendString(QString)),
             pHostTextSender, SLOT(slotSendString(QString)));
    emit sendString("<request>");
}

void ChannelHandler::setHostMediaSocket(SocketThread* socketThread) {
    pHostMediaSender = socketThread;
    connect(pHostMediaSender, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotMediaDataReady(QByteArray)));
    connect(pHostMediaSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconected()));
    songCounter = 0;
}

void ChannelHandler::addMediaSocket(const QString& userName, SocketThread* socketThread) {
    SocketThread* pUserSocket = socketThread;
    connect(this, SIGNAL(sendSkip(QString)),
            pUserSocket, SLOT(slotSendString(QString)));
    connect(this, SIGNAL(sendMessage(QString)),
            pUserSocket, SLOT(slotSendString(QString)));
    connect(pUserSocket, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotUserDisconnected()));
    connect(this, SIGNAL(sendData(QByteArray)),
            pUserSocket, SLOT(slotSendData(QByteArray)));
    connect(pUserSocket, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotMediaDataReady(QByteArray)));
    usersMediaSockets[userName] = pUserSocket;
    emit sendMessage("<guests:" + QString::number(usersMediaSockets.size()) + ">,<" + userName + ":connected.>");
    sendSongs(pUserSocket);
}

Channel ChannelHandler::getChannel() {
    return channel;
}

void ChannelHandler::slotTextDataReady(QByteArray data) {
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> channel;
    emit channelInfoChanged();
}

void ChannelHandler::slotMediaDataReady(QByteArray data) {
    QString description;
    QDataStream in(&data, QIODevice::ReadOnly);
    in >> description;
    QStringList identifier = (description.split(QRegExp("(<|>|:)"), QString::SkipEmptyParts));
    if (identifier[0] == "startMedia") {
        if (startTime.isEmpty())
            startTime.enqueue(identifier[1]);
        else
            startTime.first() = identifier[1];
    }

    else if (identifier[0] == "endOfMedia") {
        if (!songsQueue.isEmpty()) {
            songsQueue.dequeue();
            startTime.dequeue();
        }
    }

    else if (identifier[0] == "song") {
        QByteArray arr;
        QDataStream out1(&arr, QIODevice::WriteOnly);
        out1.setVersion(QDataStream::Qt_5_3);
        out1 << quint64(0);
        arr.append(data);
        songsQueue.enqueue(arr);
        if (startTime.size() == songsQueue.size() - 1)
            startTime.enqueue(QString());
        QByteArray addition;
        QDataStream out2(&addition, QIODevice::WriteOnly);
        out2.setVersion(QDataStream::Qt_5_3);
        QString timeStr;
        if (startTime.last().isNull())
            timeStr = "<startMedia:0>";
        else
            timeStr = "<startMedia:" + startTime.last() + ">";
        out2 << timeStr;
        arr.append(addition);
        out1.device()->seek(0);
        out1 << quint64(arr.size() - sizeof(quint64));
        emit sendNextSong(arr);
    }
    else if (identifier[0] == "skip") {
        emit sendSkip(description);
    }
    else if (identifier[0] == "msg") {
        emit sendMessage(description + ",<time:" + QTime::currentTime().toString("hh:mm:ss") + ">");
    }
}

void ChannelHandler::slotDisconected() {
    if (disconnected)
        emit channelClosed();
    disconnected = !disconnected;
}

void ChannelHandler::slotUserDisconnected() {
    QString userName;
    for (auto it = usersMediaSockets.begin(); it != usersMediaSockets.end(); it++)
        if (it.value() == sender())
            userName = it.key();
    usersMediaSockets[userName]->deleteLater();
    usersMediaSockets.remove(userName);
    emit sendMessage("<guests:" + QString::number(usersMediaSockets.size()) + ">,<" + userName + ":disconnected.>");
}

void ChannelHandler::sendSongs(SocketThread *sender) {
    for (int i = 0; i < songsQueue.size(); i++) {
        QByteArray arr(songsQueue[i]);
        QByteArray addition;
        QDataStream out1(&addition, QIODevice::WriteOnly);
        out1.setVersion(QDataStream::Qt_5_3);
        QString timeStr;
        if (startTime[i].isNull())
            timeStr = "<startMedia:0>";
        else
            timeStr = "<startMedia:" + startTime[i] + ">";
        out1 << timeStr;
        arr.append(addition);
        QDataStream out2(&arr, QIODevice::WriteOnly);
        out1.setVersion(QDataStream::Qt_5_3);
        out2.device()->seek(0);
        out2 << quint64(arr.size() - sizeof(quint64));
        emit sendData(arr);
    }
    disconnect(this, SIGNAL(sendData(QByteArray)),
               sender, SLOT(slotSendData(QByteArray)));
    connect(this, SIGNAL(sendNextSong(QByteArray)),
            sender, SLOT(slotSendData(QByteArray)));

}
