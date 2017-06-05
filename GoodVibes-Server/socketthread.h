#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <QThread>
#include "socket.h"

class Channel;

class SocketThread : public QObject
{
    Q_OBJECT
private:
    QThread socketThread;
    Socket* socket;

public:
    SocketThread(qintptr socketDescriptor, QObject *parent = 0);
    ~SocketThread();
    void setDescriptor(qintptr descriptor);

signals:
    void dataReady(QByteArray data);
    void disconnectedFromServer();
    void sendData(QByteArray data);
    void sendString(const QString& msg);
    void disconnectFromServer();
    void descriptorChanged(qintptr descriptor);

public slots:
    void slotSendData(QByteArray data);
    void slotSendString(const QString& msg);
    void slotDisconnectFromServer();

private slots:
    void slotDataReady(QByteArray data);
    void slotDisconnected();

};

#endif // SOCKETTHREAD_H
