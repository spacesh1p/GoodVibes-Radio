#include <QFileDialog>
#include <QDataStream>
#include <QtWidgets>
#include "channelwidget.h"
#include "ui_channelwidget.h"
#include "channelsettingsdialog.h"
#include "choosechannelwidget.h"
#include "channel.h"
#include "mediahandler.h"
#include "socketthread.h"

ChannelWidget::ChannelWidget(Channel* channel, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChannelWidget)
    , pChannel(channel)
{
    ui->setupUi(this);                                  // set up GUI

    QString parentName = parent->metaObject()->className();
    if (parentName == "ChooseChannelWidget")                            // if parent is ChooseChannelWidget
        pChooseChannelWidget = (ChooseChannelWidget*)parent;
    else
        pChooseChannelWidget = nullptr;

    pSettingsDialog = new ChannelSettingsDialog(pChannel, this);        // create setting dialog object
    connect(pSettingsDialog, SIGNAL(accepted()),
            this, SLOT(slotChannelCreated()));

    pMediaHandler = nullptr;
    pTextSender = nullptr;
    pMediaSender = nullptr;
    callCnt = 0;
}

ChannelWidget::~ChannelWidget()
{
    delete ui;
    delete pSettingsDialog;
    if (pMediaHandler != nullptr)
        delete pMediaHandler;
    if (pTextSender != nullptr)
        delete pTextSender;
    if (pMediaSender != nullptr)
        delete pMediaSender;
}

int ChannelWidget::openSettingsDialog() {
    return pSettingsDialog->exec();                        // open dialog window
}

Channel* ChannelWidget::getChannel() {
    return pChannel;
}

void ChannelWidget::unmute() {
    pMediaHandler->setMuted(ui->cmdVolume->isChecked());
}


void ChannelWidget::addSongButton(QCommandLinkButton* button) {
    ui->scrollLayout->addWidget(button);
}

void ChannelWidget::removeSongButton(QCommandLinkButton* button) {
    ui->scrollLayout->removeWidget(button);
}

SocketThread* ChannelWidget::getMediaSender() {
    return pMediaSender;
}

int ChannelWidget::getSliderVlaue() {
    return ui->sldVolume->value();
}

void ChannelWidget::slotOpenFiles() {
    QStringList filesList = QFileDialog::getOpenFileNames(this, "Open files", "", "*.mp3");       // open the dialog window for choosing songs *.mp3, *.wav
    for (QStringList::iterator it = filesList.begin(); it != filesList.end(); it++) {
        pMediaHandler->addSong(*it);
    }
}

void ChannelWidget::slotConnected() {
    if (!callCnt)
        ui->textView->append("Connection received.");
    if (pTextSender->isConnected() && pMediaSender->isConnected()) {
        ui->cmdAddSong->setEnabled(true);
        disconnect(ui->cmdBack, SIGNAL(clicked()),
                this, SLOT(slotRestart()));
        connect(ui->cmdBack, SIGNAL(clicked()),                                 // handle cmd Back clicked
                this, SLOT(slotBackClicked()));
        ui->cmdBack->setText("&Back");
        ui->cmdBack->setEnabled(true);
    }
    if (sender() == pTextSender)
        emit sendChannelData(*pChannel);
    callCnt++;
    if (callCnt == 2)
        callCnt = 0;
}

void ChannelWidget::slotError(const QString& strError) {
    QString str = ui->textView->toPlainText();
    if (!str.contains(strError))
        ui->textView->append(strError);
    ui->cmdAddSong->setEnabled(false);
    ui->cmdBack->setEnabled(false);
    if (pMediaSender->isConnected() || pTextSender->isConnected())
        emit disconnectFromServer();
    else {
        ui->cmdBack->setEnabled(true);
    }
}

void ChannelWidget::slotDisconnected() {
    if (!callCnt)
        ui->textView->append("Disconnected from server.");

    if (!pTextSender->isConnected() && !pMediaSender->isConnected()) {
        ui->cmdAddSong->setEnabled(false);
        ui->cmdBack->setText("&Restart");
        disconnect(ui->cmdBack, SIGNAL(clicked()),
                this, SLOT(slotBackClicked()));
        connect(ui->cmdBack, SIGNAL(clicked()),
                this, SLOT(slotRestart()));
        ui->cmdBack->setEnabled(true);
    }
    callCnt++;
    if (callCnt == 2)
        callCnt = 0;
}

void ChannelWidget::slotDataReady(QByteArray data) {
    QString description;
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_3);
    in >> description;
    QStringList descriptList = description.split(",");
    QStringList identifier;
    if (!descriptList.isEmpty())
        identifier = descriptList[0].split(QRegExp("(<|>|:)"), QString::SkipEmptyParts);
    if (!identifier.isEmpty()) {
        if (identifier[0] == "guests")
            ui->lcdNumber->display(identifier[1]);
    }
}

void ChannelWidget::slotRestart() {
    emit connectToServer();
}

void ChannelWidget::slotBackClicked() {
    pMediaHandler->setMuted(true);
    if (pChooseChannelWidget != nullptr)
        pChooseChannelWidget->backToChooseChannel();
}

void ChannelWidget::slotFileNotOpened() {
    ui->textView->append("Cannot open file.");
}

void ChannelWidget::slotChannelCreated() {
    disconnect(pSettingsDialog, SIGNAL(accepted()),
            this, SLOT(slotChannelCreated()));
    pTextSender = new SocketThread();                                                                   // create TextSender
    pTextSender->setDescription(QString("<name:" + pChannel->getName() + ">,<purpose:sendChannelInfo>"));
    pMediaSender = new SocketThread();                                                                  // create MediaSender
    pMediaSender->setDescription(QString("<name:" + pChannel->getName() + ">,<for:sendMedia>"));

    connect(pTextSender, SIGNAL(connectedToServer()),
            this, SLOT(slotConnected()));
    connect(pTextSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    connect(pTextSender, SIGNAL(connectionError(QString)),
            this, SLOT(slotError(QString)));
    connect(pTextSender, SIGNAL(fileNotOpened()),
            this, SLOT(slotFileNotOpened()));
    connect(pTextSender, SIGNAL(dataReady(QByteArray)),
            this, SLOT(slotDataReady(QByteArray)));
    connect(this, SIGNAL(connectToServer()),
            pTextSender, SLOT(slotConnectToServer()));
    connect(this, SIGNAL(disconnectFromServer()),
            pTextSender, SLOT(slotDisconnectFromServer()));
    connect(this, SIGNAL(sendChannelData(Channel)),
            pTextSender, SLOT(slotSendChannelData(Channel)));
    connect(pMediaSender, SIGNAL(connectedToServer()),
            this, SLOT(slotConnected()));
    connect(pMediaSender, SIGNAL(disconnectedFromServer()),
            this, SLOT(slotDisconnected()));
    connect(pMediaSender, SIGNAL(connectionError(QString)),
            this, SLOT(slotError(QString)));
    connect(pMediaSender, SIGNAL(fileNotOpened()),
            this, SLOT(slotFileNotOpened()));
    connect(this, SIGNAL(connectToServer()),
            pMediaSender, SLOT(slotConnectToServer()));
    connect(this, SIGNAL(disconnectFromServer()),
            pMediaSender, SLOT(slotDisconnectFromServer()));

    emit connectToServer();

    pMediaHandler = new MediaHandler(this);                                                             // create MediaHandler object


    ui->cmdAddSong->setEnabled(false);
    ui->cmdBack->setEnabled(false);
    ui->sldVolume->setValue(50);

    connect(ui->cmdAddSong, SIGNAL(clicked()),                              // handle Add Song button clicked
            this, SLOT(slotOpenFiles()));
    connect(ui->cmdTurnOff, SIGNAL(clicked()),
            pChooseChannelWidget, SLOT(slotTurnOffChannel()));
    connect(ui->cmdVolume, SIGNAL(clicked(bool)),
            pMediaHandler, SLOT(slotMutedChanged(bool)));
    connect(ui->sldVolume, SIGNAL(valueChanged(int)),
            pMediaHandler, SLOT(slotChangeVolume(int)));

    ui->channelName->setText(pChannel->getName());
    ui->lcdNumber->display(0);
}

void ChannelWidget::updateData() {
    emit sendChannelData(*pChannel);
}

//void ChannelWidget::slotRestart() {
//    if (pTextSocket->state() != QTcpSocket::ConnectedState)
//        pTextSocket->connectToHost(ServerInfo::strHost, ServerInfo::nPort);
//    if (pMediaSocket->state() != QTcpSocket::ConnectedState)
//        pMediaSocket->connectToHost(ServerInfo::strHost, ServerInfo::nPort);
//}

//void ChannelWidget::slotReadClient() {
//    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
//    QDataStream in(pClientSocket);
//    in.setVersion(QDataStream::Qt_5_3);
//    while(true) {
//        if (!nextBlockSize) {
//            if (pClientSocket->bytesAvailable() < sizeof(quint16))
//                break;
//            in >> nextBlockSize;
//        }

//        if (pClientSocket->bytesAvailable() < nextBlockSize)
//            break;

//        QString msg;
//        in >> msg;
//        if (msg == "next") {
//            if (pSongQueue->isEmpty()) {                                                // if there is no songs
//                sendToClient(pClientSocket, "The channel ran out of music :(");
//                break;
//            }
//            QFile* pFile(pSongQueue->dequeue());
//            sendToClient(pClientSocket, pFile);                                         // send audio file
//        }

//        if (msg == "done") {
//            sendToClient(pClientSocket, "time/" + /*current playing time*/);            // send playing time
//        }
//    }
//}
//void ChannelWidget::slotNewConnection() {
//    QTcpSocket* pClientSocket = pServer->nextPendingConnection();          // get new connection socket
//    connect(pClientSocket, SIGNAL(disconnected()),                         // handle disconnection
//            this, SLOT(slotDisconnetion()));

//    connect(pClientSocket, SIGNAL(readyRead()),                            // handle reading the socket
//            this, SLOT(slotReadClient()));

//    clientsSockets.push_back(pClientSocket);                               // add new socket to the list
//    ui->textView->append("New listener connected.");
//    ui->lcdNumber->display(clientsSockets.size());

//    sendToClient(pClientSocket, "Welcome to " + channelWelcome + "!");
//}

//void ChannelWidget::slotDisconnetion() {
//    QTcpSocket* pSender = (QTcpSocket*)sender();
//    clientsSockets.removeOne(pSender);                                     // delete the disconnected socket
//    ui->textView->append("Listener disconnected.");
//    ui->lcdNumber->display(clientsSockets.size());
//    pSender->deleteLater();
//}
