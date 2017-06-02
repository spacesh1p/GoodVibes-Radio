#include <QFile>
#include <QDataStream>
#include "socket.h"
#include "channel.h"

Socket::Socket(qintptr socketDescriptor, QObject *parent)
    : QObject(parent)
{
    pSocket = new QTcpSocket(this);

    connect(pSocket, SIGNAL(disconnected()),
            this, SLOT(slotDisconnected()));
    connect(pSocket, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));

    pSocket->setSocketDescriptor(socketDescriptor);
}

Socket::~Socket() {
    pSocket->close();
    delete pSocket;
}

void Socket::slotSetDescriptor(qintptr descriptor) {
    pSocket->setSocketDescriptor(descriptor);
}


void Socket::slotDisconnected() {
    emit disconnectedFromServer();
}

void Socket::slotSendData(QByteArray data) {
    pSocket->write(arrBlock);
    pSocket->flush();
}

void Socket::slotSendString(const QString& msg) {
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint64(0) << msg;                                                  // convert String into bytes
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));
    pSocket->write(arrBlock);
    pSocket->flush();
}

void Socket::slotDisconnectFromServer() {
    pSocket->disconnectFromHost();
}

void Socket::slotReadyRead() {
    QDataStream in(pSocket);
    in.setVersion(QDataStream::Qt_5_3);

    while (true) {
        if (!nextBlockSize) {
            if (pSocket->bytesAvailable() < (qint64)sizeof(qint64))
                break;
            in >> nextBlockSize;
        }

        if(pSocket->bytesAvailable() < (qint64)nextBlockSize)
            break;

        QByteArray data = pSocket->read(nextBlockSize);

        emit dataReady(data);
        nextBlockSize = 0;
    }
}
