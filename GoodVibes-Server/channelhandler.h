#ifndef CHANNELHANDLER_H
#define CHANNELHANDLER_H

#include <QObject>
#include <QMap>
#include <QQueue>
#include "channel.h"

class SocketThread;
class QTime;
class Server;

class ChannelHandler : public QObject
{
    Q_OBJECT

private:
    Server* pServer;
    Channel channel;
    SocketThread* pHostTextSender;
    SocketThread* pHostMediaSender;
    QMap<QString, SocketThread*> usersMediaSockets;
    QQueue<QByteArray> songsQueue;
    QQueue<QString> startTime;
    int songCounter;
    bool disconnected;

    void sendSongs(SocketThread* sender);
    void sendChannelsList(SocketThread* sender);

public:
    ChannelHandler(QObject *parent = 0);
    ~ChannelHandler();
    void setHostTextSocket(SocketThread* socketThread);
    void setHostMediaSocket(SocketThread* socketThread);
    void addMediaSocket(const QString& userName, SocketThread* socketThread);
    Channel getChannel();

signals:
    void sendData(QByteArray data);
    void sendString(const QString& msg);
    void channelClosed();
    void sendMessage(const QString& strNum);
    void sendNextSong(QByteArray data);
    void channelInfoChanged();
    void sendSkip(const QString& skip);

public slots:
    void slotDisconected();
    void slotTextDataReady(QByteArray data);
    void slotMediaDataReady(QByteArray data);
    void slotUserDisconnected();
};

#endif // CHANNELHANDLER_H
