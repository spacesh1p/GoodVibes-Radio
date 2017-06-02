#ifndef CHANNELHANDLER_H
#define CHANNELHANDLER_H

#include <QObject>
#include <QMap>
#include <QQueue>

class SocketThread;
class Channel;
class QTime;
class Server;

class ChannelHandler : public QObject
{
    Q_OBJECT

private:
    Server* pServer;
    Channel channel;
    QString hostName;
    SocketThread* pHostTextSender;
    SocketThread* pHostMediaSender;
    QMap<QString, SocketThread*> usersTextSocets;
    QMap<QString, SocketThread*> usersMediaSockets;
    QQueue<QByteArray> songsQueue;
    QQueue<QTime> arravingTime;
    int songCounter;
    bool disconnected;

    void sendSongs(SocketThread* sender);
    void sendChannelsList(SocketThread* sender);

public:
    ChannelHandler(const QString& host, QObject *parent = 0);
    ~ChannelHandler();
    void setHostTextSocket(qintptr socketDescriptor);
    void setHostMediaSocket(qintptr socketDescriptor);
    void addTextSocket(const QString& userName, quintptr socketDescriptor);
    void addMediaSocket(const QString& userName, quintptr socketDescriptor);
    QString getHostName();

signals:
    void channelClosed();
    void channelSeted(const QString& channelName);
    void sendNumOfGuests(const QString& strNum);
    void sendNextSong(QByteArray data);

public slots:
    void slotDisconected();
    void slotTextDataReady(QByteArray data);
    void slotMediaDataReady(QByteArray data);
    void slotRequestReady(QByteArray data);
    void slotUserDisconnected();
};

#endif // CHANNELHANDLER_H
