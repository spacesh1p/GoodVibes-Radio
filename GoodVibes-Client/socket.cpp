#include <QFile>
#include <QDataStream>
#include "socket.h"
#include "channel.h"
#include "serverinfo.h"

Socket::Socket(QObject *parent) : QObject(parent)
{
    pSocket = new QTcpSocket(this);
    connect(pSocket, SIGNAL(connected()),
            this, SLOT(slotConnected()));
    connect(pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError)));
    connect(pSocket, SIGNAL(disconnected()),
            this, SLOT(slotDisconnected()));
    connect(pSocket, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
    nextBlockSize = 0;
    if (!socketDescription.isEmpty())
        pSocket->connectToHost(ServerInfo::strHost, ServerInfo::nPort);
}

Socket::~Socket() {
    pSocket->disconnectFromHost();
    pSocket->deleteLater();
}

void Socket::slotSetDescription(const QString& description) {
    socketDescription = description;
}

void Socket::sendDescription() {
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint64(0) << socketDescription;
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));
    pSocket->write(arrBlock);
    qDebug() << "description sended";
    pSocket->flush();
    emit connectedToServer();

}

void Socket::slotError(QAbstractSocket::SocketError err) {
    QString strError =
            "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                         "The host was not found." :
                         err == QAbstractSocket::RemoteHostClosedError ?
                         "The remote host is closed." :
                         err == QAbstractSocket::ConnectionRefusedError ?
                         "The connection was refused." :
                         QString(pSocket->errorString()));
    emit connectionError(strError);
}

void Socket::slotConnected() {
    sendDescription();
}

void Socket::slotDisconnected() {
    emit disconnectedFromServer();
}

void Socket::slotSendChannelData(const Channel& channel) {
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint64(0) << channel;                                                  // convert Channel into bytes
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));
    pSocket->write(arrBlock);
    pSocket->flush();
}

void Socket::slotSendFileData(const QString& songName, const QString& path) {
    QFile pFile(path);
    if (!pFile.open(QFile::ReadOnly))                                              // open audio file
        {
            emit fileNotOpened();                                                  // signal that file wasn't opened
            return;
        }
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    qDebug() << "song name: " << songName;
    QString strInfo = "<song>,<name:" + songName + ">";
    out << quint64(0) << strInfo << pFile.readAll();             // convert file data into bytes
    out.device()->seek(0);
    out << quint64(arrBlock.size() - sizeof(quint64));
    pFile.close();
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

void Socket::slotConnectToServer() {
    pSocket->connectToHost(ServerInfo::strHost, ServerInfo::nPort);
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
            qDebug() << "nbs: " << nextBlockSize;
        }

        qDebug() << "bytes available: " << pSocket->bytesAvailable();
        if(pSocket->bytesAvailable() < (qint64)nextBlockSize)
            break;

        QByteArray data = pSocket->read(nextBlockSize);

        emit dataReady(data);
        nextBlockSize = 0;
    }
}
