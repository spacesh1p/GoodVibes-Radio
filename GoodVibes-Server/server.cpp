#include <QTcpSocket>
#include <QDataStream>
#include <QtDebug>
#include "server.h"
#include "socketthread.h"
#include "channelhandler.h"
#include "channel.h"

Server::Server(int nPort, QObject *parent)
    : QTcpServer(parent)
{
    if (!this->listen(QHostAddress::Any, nPort)) {
            qDebug() << "Unable to start the server:" + this->errorString();
            this->close();
            return;
        }
}

void Server::incomingConnection(qintptr socketDescriptor) {
    SocketThread* pSocketThread = new SocketThread(socketDescriptor);
    connect(pSocketThread, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotReadDescription(QByteArray)));
    connect(pSocketThread, SIGNAL(disconnectedFromServer()),
            pSocketThread, SLOT(deleteLater()));
}

void Server::start() {
    while (true);
}

void Server::slotReadDescription(QByteArray data) {
    SocketThread* pClient = (SocketThread*)sender();
    disconnect(pClient, SIGNAL(dataReady(QByteArray)),
               this, SLOT(slotReadDescription(QByteArray)));

    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    QString description;
    in >> description;
    qDebug() << description;
    QStringList descriptList = description.split(",", QString::SkipEmptyParts);
    QStringList identifier;
    if (!descriptList.isEmpty())
        identifier = descriptList[0].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts);
    if (!identifier.isEmpty()) {
        if (identifier[0] == "host") {
            QString chName = (descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
            ChannelHandler *pChHandler;
            if (!channelsMap[identifier[1]].contains(chName)) {
                pChHandler = new ChannelHandler(this);
                connect(pChHandler, SIGNAL(channelClosed()),
                        this, SLOT(slotChannelClosed()));
                connect(pChHandler, SIGNAL(channelInfoChanged()),
                        this, SLOT(slotChannelInfoChanged()));
                (channelsMap[identifier[1]])[chName] = pChHandler;
            }
            else
                pChHandler = (channelsMap[identifier[1]])[chName];
            QString purposeStr = (descriptList[2].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
            if (purposeStr == "sendChannelInfo")
                pChHandler->setHostTextSocket(pClient);
            else if (purposeStr == "sendMedia")
                pChHandler->setHostMediaSocket(pClient);
            disconnect(pClient, SIGNAL(disconnectedFromServer()),
                       pClient, SLOT(deleteLater()));
        }
        else if (identifier[0] == "user") {
            if (descriptList.size() == 4) {
                QString hostStr = (descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
                QString chName = (descriptList[2].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
                QString purposeStr = (descriptList[3].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
                if (channelsMap.contains(hostStr)) {
                    if (channelsMap[hostStr].contains(chName)) {
                        if (purposeStr == "readMedia") {
                            (channelsMap[hostStr])[chName]->addMediaSocket(identifier[1], pClient);
                            disconnect(pClient, SIGNAL(disconnectedFromServer()),
                                       pClient, SLOT(deleteLater()));
                        }
                    }
                    else
                        pClient->deleteLater();
                }
                else
                    pClient->deleteLater();

            }
            else if (descriptList.size() == 2) {
                QString purposeStr = (descriptList[1].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts))[1];
                if (purposeStr == "readChannelsInfo") {
                    connect(pClient, SIGNAL(dataReady(QByteArray)),
                            this, SLOT(slotRequestReady(QByteArray)));
                    connect(pClient, SIGNAL(disconnectedFromServer()),
                            this, SLOT(slotDisconnected()));
                    usersTextSocets[identifier[1]] = pClient;
                    disconnect(pClient, SIGNAL(disconnectedFromServer()),
                               pClient, SLOT(deleteLater()));
                }
            }
        }
        else if (identifier[0] == "username") {
            connect(pClient, SIGNAL(dataReady(QByteArray)),
                    this, SLOT(slotReadUserName(QByteArray)));
        }
        else {
            pClient->deleteLater();
        }
    }
    else
        pClient->deleteLater();
}

void Server::slotReadUserName(QByteArray data) {
    SocketThread* pClient = (SocketThread*)sender();
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    QString username;
    in >> username;
    bool accepted = !usersTextSocets.contains(username);
    QByteArray newData;
    QDataStream out(&newData, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint64(0) << accepted;
    out.device()->seek(0);
    out << quint64(newData.size() - sizeof(quint64));
    pClient->slotSendData(newData);
}

QString Server::findSocketUser(SocketThread* socket) {
    for (auto it = usersTextSocets.begin(); it != usersTextSocets.end(); it++)
        if (it.value() == socket)
            return it.key();
    return QString();
}

void Server::slotRequestReady(QByteArray data) {
    SocketThread* pClient = (SocketThread*)sender();
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    QString recievedStr;
    in >> recievedStr;
    if (recievedStr == "<request>") {
        QList<Channel> channelsList;
        QString userStr = findSocketUser(pClient);
        for (auto it = channelsMap.begin(); it != channelsMap.end(); it++) {
            if (it.key() != userStr) {
                for (auto itr = (*it).begin(); itr != (*it).end(); itr++)
                    channelsList.append(itr.value()->getChannel());
            }
        }
        QByteArray newData;
        QDataStream out(&newData, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_3);
        out << quint64(0) << channelsList;
        out.device()->seek(0);
        out << quint64(newData.size() - sizeof(quint64));
        pClient->slotSendData(newData);
    }
}

void Server::slotDisconnected() {
    SocketThread* pClient = (SocketThread*)sender();
    QString userStr = findSocketUser(pClient);
    usersTextSocets.remove(userStr);
    pClient->deleteLater();
}

void Server::slotChannelInfoChanged() {
    ChannelHandler* pChHandler = (ChannelHandler*)sender();
    QString hostName = pChHandler->getChannel().getHostName();
    QString key;
    for (auto it = channelsMap[hostName].begin(); it != channelsMap[hostName].end(); it++)
        if (it.value() == pChHandler) {
            key = it.key();
            break;
        }
    channelsMap[hostName].remove(key);
    (channelsMap[hostName])[pChHandler->getChannel().getChannelName()] = pChHandler;
}

void Server::slotChannelClosed() {
    ChannelHandler* pChHandler = (ChannelHandler*)sender();
    QString hostStr = pChHandler->getChannel().getHostName();
    QString chNameStr = pChHandler->getChannel().getChannelName();
    channelsMap[hostStr].remove(chNameStr);
    if (channelsMap[hostStr].isEmpty())
        channelsMap.remove(hostStr);
    pChHandler->deleteLater();
}
