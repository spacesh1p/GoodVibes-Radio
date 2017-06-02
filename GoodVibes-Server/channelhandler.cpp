#include <QDataStream>
#include <QTime>
#include <socketthread.h>
#include "channelhandler.h"
#include <channel.h>
#include <server.h>

ChannelHandler::ChannelHandler(const QString& host, QObject *parent)
    : QObject(parent)
    , hostName(host)
{

    QString parentClass = parent->metaObject()->className();
    if(parentClass == "Server")
        pServer = (Server*)parent;
    else
        pServer = nullptr;

    pHostTextSender = nullptr;
    pHostMediaSender = nullptr;
    songsCounter = 0;
    disconnected = false;
}

ChannelHandler::~ChannelHandler() {
    if (pHostTextSender != nullptr)
        delete pHostTextSender;
    if (pHostMediaSender != nullptr)
        delete pHostMediaSender;
}

void ChannelHandler::setHostTextSocket(qintptr socketDescriptor) {
    pHostTextSender = new SocketThread(descriptor);
    connect(pHostTextSender, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotTextDataReady(QByteArray)));
    connect(pHostTextSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconected()));
    connect(this, SIGNAL(sendNumOfGuests(QString)),
            pHostTextSender, SLOT(slotSendString(QString)));
    pHostTextSender->slotSendString("<request>");
}

void ChannelHandler::setHostMediaSocket(qintptr socketDescriptor) {
    pHostMediaSender->setDescriptor(descriptor);
    connect(pHostMediaSender, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotMediaDataReady(QByteArray)));
    connect(pHostMediaSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconected()));
    songCounter = 0;
}

void ChannelHandler::addTextSocket(const QString& userName, quintptr socketDescriptor) {
    SocketThread* pUserSocket = new SocketThread(socketDescriptor);
    connect(this, SIGNAL(sendNumOfGuests(QString)),
            pUserSocket, SLOT(slotSendString(QString)));
    connect(pUserSocket, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotRequestReady(QByteArray)));
    usersTextSocets[userName] = pUserSocket;
    sendChannelsList(pUserSocket);
}

void ChannelHandler::addMediaSocket(const QString& userName, quintptr socketDescriptor) {
    SocketThread* pUserSocket = new SocketThread(socketDescriptor);
    connect(pUserSocket, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotUserDisconnected()));
    usersMediaSockets[userName] = pUserSocket;
    emit sendNumOfGuests("<guests:" + QString::number(usersMediaSockets.size()) + ">");
    sendSongs(pUserSocket);
}

QString ChannelHandler::getHostName() {
    return hostName;
}

void ChannelHandler::slotTextDataReady(QByteArray data) {
    QDataStream in(&data, QIODevice::WriteOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> channel;
    emit channelSeted(channel.getName());
}

void ChannelHandler::slotRequestReady(QByteArray data) {
    QDataStream in(&data, QIODevice::WriteOnly);
    in.setVersion(QDataStream::Qt_5_3);
    QString recievedStr;
    in >> recievedStr;
    if (recievedStr = "<request>")
        sendChannelsList(sender());
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
    sendNextSong(arr);
}

void ChannelHandler::slotDisconected() {
    if (disconnected)
        emit channelClosed();
    disconnected = !disconnected;
}

void ChannelHandler::slotUserDisconnected() {
    QString userName;
    for (auto it = usersMediaSockets.begin(); it != usersMediaSockets.end(); it++)
        if (*it.value() == sender())
            userName = *it.key();
    delete usersMediaSockets[userName];
    delete usersTextSocets[userName];
    usersMediaSockets.remove(userName);
    usersTextSocets.remove(userName);
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
        sender->slotSendData(arr);
    }
    connect(this, SIGNAL(sendNextSong(QByteArray)),
            sender, SLOT(slotSendData(QByteArray)));

}

void ChannelHandler::sendChannelsList(SocketThread *sender) {
    QList<Channel> channelList;
    if (pServer != nullptr)
        channelList = pServer->getChannelsList(hostName);
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint64(0) << channelList;
    out.device()->seek(0);
    out << quint64(data.size() - sizeof(quint64));
    sender->sendData(data);
}
