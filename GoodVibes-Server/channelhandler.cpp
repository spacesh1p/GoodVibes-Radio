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
        delete pHostTextSender;
    if (pHostMediaSender != nullptr)
        delete pHostMediaSender;
    for (auto it = usersMediaSockets.begin(); it != usersMediaSockets.end(); it++)
        delete it.value();
}

void ChannelHandler::setHostTextSocket(SocketThread* socketThread) {
    pHostTextSender = socketThread;
    connect(pHostTextSender, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotTextDataReady(QByteArray)));
    connect(pHostTextSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconected()));
    connect(this, SIGNAL(sendNumOfGuests(QString)),
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
    connect(this, SIGNAL(sendNumOfGuests(QString)),
            pUserSocket, SLOT(slotSendString(QString)));
    connect(pUserSocket, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotUserDisconnected()));
    connect(this, SIGNAL(sendData(QByteArray)),
            pUserSocket, SLOT(slotSendData(QByteArray)));
    usersMediaSockets[userName] = pUserSocket;
    emit sendNumOfGuests("<guests:" + QString::number(usersMediaSockets.size()) + ">");
    sendSongs(pUserSocket);
}

Channel ChannelHandler::getChannel() {
    return channel;
}

void ChannelHandler::slotTextDataReady(QByteArray data) {
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> channel;
}

void ChannelHandler::slotMediaDataReady(QByteArray data) {
    if (!songsQueue.isEmpty()) {
        songsQueue.dequeue();
        arravingTime.dequeue();
    }
    QTime time = QTime::currentTime();
    arravingTime.enqueue(time);
    QByteArray arr(data);
    QByteArray addition;
    QDataStream out(&addition, QIODevice::WriteOnly);
    QString posStr = "<pos:" + QString::number(time.msecsTo(QTime::currentTime())) + ">";
    out << posStr;
    arr.append(addition);
    emit sendNextSong(arr);
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
    delete usersMediaSockets[userName];
    usersMediaSockets.remove(userName);
    emit sendNumOfGuests("<guests:" + QString::number(usersMediaSockets.size()) + ">");
}

void ChannelHandler::sendSongs(SocketThread *sender) {
    for (int i = 0; i < songsQueue.size(); i++) {
        QByteArray arr(songsQueue[i]);
        QByteArray addition;
        QDataStream out(&addition, QIODevice::WriteOnly);
        QTime time = arravingTime[i];
        QString posStr = "<pos:" + QString::number(time.msecsTo(QTime::currentTime())) + ">";
        out << posStr;
        arr.append(addition);
        emit sendData(arr);
    }
    disconnect(this, SIGNAL(sendData(QByteArray)),
               sender, SLOT(slotSendData(QByteArray)));
    connect(this, SIGNAL(sendNextSong(QByteArray)),
            sender, SLOT(slotSendData(QByteArray)));

}
