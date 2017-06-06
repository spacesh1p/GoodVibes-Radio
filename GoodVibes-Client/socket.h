#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QTcpSocket>

class Channel;

class Socket : public QObject
{
    Q_OBJECT
private:
    QTcpSocket* pSocket;
    QString socketDescription;
    quint64 nextBlockSize;

    void sendDescription();

public:
    Socket(QObject *parent = 0);
    ~Socket();
    void setDescription(const QString& description);

signals:
    void connectionError(const QString& strError);
    void fileNotOpened();
    void connectedToServer();
    void disconnectedFromServer();
    void dataReady(QByteArray data);

private slots:
    void slotConnectToServer();
    void slotDisconnectFromServer();
    void slotSetDescription(const QString& description);
    void slotError(QAbstractSocket::SocketError);
    void slotConnected();
    void slotDisconnected();
    void slotSendChannelData(const Channel& channel);
    void slotSendFileData(const QString& songName, const QString& path);
    void slotReadyRead();
    void slotSendString(const QString& msg);
};

#endif // SOCKET_H
