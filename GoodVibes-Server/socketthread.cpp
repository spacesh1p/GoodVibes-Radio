#include "socketthread.h"
#include "channel.h"

SocketThread::SocketThread(qintptr socketDescriptor, QObject *parent)
    : QObject(parent)
{
    socket = new Socket(socketDescriptor);
    socket->moveToThread(&socketThread);
    qRegisterMetaType<Channel>("Channel");
    connect(&socketThread, SIGNAL(finished()),
            socket, SLOT(deleteLater()));
    connect(this, SIGNAL(descriptorChanged(qintptr)),
            socket, SLOT(slotSetDescriptor(qintptr)));
    connect(this, SIGNAL(sendData(QByteArray)),
            socket, SLOT(slotSendData(QByteArray)));
    connect(this, SIGNAL(sendString(QString)),
            socket, SLOT(slotSendString(QString)));
    connect(this, SIGNAL(disconnectFromServer()),
            socket, SLOT(slotDisconnectFromServer()));
    connect(socket, SIGNAL(disconnectedFromServer()),
              this, SLOT(slotDisconnected()));
    connect(socket, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotDataReady(QByteArray)));

    socketThread.start();
}

SocketThread::~SocketThread() {
    socketThread.quit();
    socketThread.wait();
}

void SocketThread::setDescriptor(qintptr descriptor) {
    emit descriptorChanged(descriptor);
}

void SocketThread::slotDataReady(QByteArray data) {
    emit dataReady(data);
}

void SocketThread::slotSendData(QByteArray data) {
    emit sendData(data);
}

void SocketThread::slotSendString(const QString& msg) {
    emit sendString(msg);
}

void SocketThread::slotDisconnected() {
    emit disconnectedFromServer();
}

void SocketThread::slotDisconnectFromServer() {
    emit disconnectFromServer();
}
