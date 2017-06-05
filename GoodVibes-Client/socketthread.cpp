#include "socketthread.h"
#include "channel.h"

SocketThread::SocketThread() {
    socket = new Socket();
    socket->moveToThread(&socketThread);
    qRegisterMetaType<Channel>("Channel");
    connect(&socketThread, SIGNAL(finished()),
            socket, SLOT(deleteLater()));
    connect(this, SIGNAL(descriptionChanged(QString)),
            socket, SLOT(slotSetDescription(QString)));
    connect(this, SIGNAL(sendChannelData(Channel)),
            socket, SLOT(slotSendChannelData(Channel)));
    connect(this, SIGNAL(sendFileData(QString, QString)),
            socket, SLOT(slotSendFileData(QString, QString)));
    connect(this, SIGNAL(connectToServer()),
            socket, SLOT(slotConnectToServer()));
    connect(this, SIGNAL(disconnectFromServer()),
            socket, SLOT(slotDisconnectFromServer()));
    connect(this, SIGNAL(sendString(QString)),
            socket, SLOT(slotSendString(QString)));
    connect(socket, SIGNAL(connectedToServer()),
            this, SLOT(slotConnected()));
    connect(socket, SIGNAL(disconnectedFromServer()),
              this, SLOT(slotDisconnected()));
    connect(socket, SIGNAL(connectionError(QString)),
              this, SLOT(slotError(QString)));
    connect(socket, SIGNAL(fileNotOpened()),
            this, SLOT(slotFileNotOpened()));
    connect(socket, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotDataReady(QByteArray)));

    connectStatus = false;

    socketThread.start();
}

SocketThread::~SocketThread() {
    socketThread.quit();
    socketThread.wait();
}

void SocketThread::setDescription(const QString& description) {
    emit descriptionChanged(description);
}

void SocketThread::slotDataReady(QByteArray data) {
    emit dataReady(data);
}

void SocketThread::slotSendChannelData(const Channel& channel) {
    emit sendChannelData(channel);
}

void SocketThread::slotSendFileData(const QString& songName, const QString& path) {
    emit sendFileData(songName, path);
}

bool SocketThread::isConnected(){
    return connectStatus;
}

void SocketThread::slotError(const QString& strError) {
    emit connectionError(strError);
}

void SocketThread::slotConnected() {
    connectStatus = true;
    emit connectedToServer();
}

void SocketThread::slotDisconnected() {
    connectStatus = false;
    emit disconnectedFromServer();
}
void SocketThread::slotConnectToServer() {
    emit connectToServer();
}

void SocketThread::slotDisconnectFromServer() {
    emit disconnectFromServer();
}

void SocketThread::slotFileNotOpened() {
    emit fileNotOpened();
}
void SocketThread::slotSendString(const QString& msg) {
    emit sendString(msg);
}
