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
    quint64 nextBlockSize;

public:
    Socket(qintptr socketDescriptor, QObject *parent = 0);
    ~Socket();
    void setDescriptor(qintptr descriptor);

signals:
    void disconnectedFromServer();
    void dataReady(QByteArray data);

public slots:
    void slotDisconnectFromServer();
    void slotSetDescriptor(qintptr descriptor);
    void slotDisconnected();
    void slotSendData(QByteArray data);
    void slotSendString(const QString& msg);
    void slotReadyRead();
};

#endif // SOCKET_H
