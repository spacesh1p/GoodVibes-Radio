#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>

class ChannelHandler;
class Channel;
class SocketThread;

class Server : public QTcpServer
{
    Q_OBJECT

private:
    int port;
    QMap<QString, QMap<QString, ChannelHandler*>> channelsMap;
    QMap<QString, SocketThread*> usersTextSocets;

    QString findSocketUser(SocketThread* socket);

public:
    Server(int nPort, QObject *parent = 0);
    QList<Channel> getChannelsList(const QString& hostName);   // return list of channels whose host's name is not equal to hostName
    void start();

protected:
    void incomingConnection(qintptr socketDescriptor);


private slots:
    void slotReadDescription(QByteArray data);
    void slotChannelClosed();
    void slotRequestReady(QByteArray data);
    void slotDisconnected();
    void slotReadUserName(QByteArray data);
    void slotChannelInfoChanged();

};

#endif // SERVER_H
