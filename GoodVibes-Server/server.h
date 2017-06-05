#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>

class ChannelHandler;
class Channel;
class SocketThread;

class Server : public QObject
{
    Q_OBJECT

private:
    QTcpServer* pServer;
    QMap<QString, QMap<QString, ChannelHandler*>> channelsMap;
    QMap<QString, SocketThread*> usersTextSocets;

    QString findSocketUser(SocketThread* socket);

public:
    Server(int nPort, QObject *parent = 0);
    ~Server();
    QList<Channel> getChannelsList(const QString& hostName);   // return list of channels whose host's name is not equal to hostName
    void start();

private slots:
    void slotNewConnection();
    void slotReadDescription(QByteArray data);
    void slotChannelClosed();
    void slotRequestReady(QByteArray data);
    void slotDisconnected();
    void slotReadUserName(QByteArray data);

};

#endif // SERVER_H
