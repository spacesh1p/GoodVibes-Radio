#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <QThread>
#include "socket.h"

class QFile;
class Channel;

class SocketThread : public QObject
{
    Q_OBJECT
private:
    QThread socketThread;
    Socket* socket;
    bool connectStatus;

public:
    SocketThread();
    ~SocketThread();
    bool isConnected();

    void setDescription(const QString& description);

signals:
    void dataReady(QByteArray data);
    void connectionError(const QString& strError);
    void fileNotOpened();
    void connectedToServer();
    void disconnectedFromServer();
    void sendChannelData(const Channel& channel);
    void sendFileData(const QString& songName, const QString& path);
    void connectToServer();
    void disconnectFromServer();
    void descriptionChanged(const QString& description);
    void sendString(const QString& msg);

private slots:
    void slotDataReady(QByteArray data);
    void slotConnectToServer();
    void slotDisconnectFromServer();
    void slotError(const QString& strError);
    void slotConnected();
    void slotDisconnected();
    void slotSendChannelData(const Channel& channel);
    void slotSendFileData(const QString& songName, const QString& path);
    void slotFileNotOpened();
    void slotSendString(const QString& msg);
};

#endif // SOCKETTHREAD_H
