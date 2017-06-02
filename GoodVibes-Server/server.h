#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>

class ChannelHandler;
class Channel;

class Server : public QObject
{
    Q_OBJECT

private:
    QTcpServer* pServer;
    QMap<QString, QList<ChannelHandler*>> channelsMap;

public:
    Server(QObject *parent = 0);
    ~Server();
    QList<Channel> getChannelsList(const QString& hostName);   // return list of channels whose host's name is not equal to hostName

signals:

private slots:


};

#endif // SERVER_H
