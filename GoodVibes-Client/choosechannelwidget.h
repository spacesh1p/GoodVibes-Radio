#ifndef CHOOSECHANNELWIDGET_H
#define CHOOSECHANNELWIDGET_H

#include <QWidget>

namespace Ui {
class ChooseChannelWidget;
}

class ChannelWidget;
class QCommandLinkButton;
class MainWindow;
class SocketThread;
class PlayerWidget;
class Channel;
class CheckingPasswdDialog;


class ChooseChannelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChooseChannelWidget(QWidget *parent = 0);
    ~ChooseChannelWidget();
    void backToChooseChannel();

private:
    Ui::ChooseChannelWidget *ui;
    MainWindow* pMainWindow;
    QList<ChannelWidget*> hostChannelsList;
    QList<Channel*> guestChannelsList;
    QList<QCommandLinkButton*> hostButtons;
    QList<QCommandLinkButton*> guestButtons;
    QPair<ChannelWidget*, QCommandLinkButton*> pairChoosenHostChannel;
    QPair<Channel*, QCommandLinkButton*> pairChoosenGuestChannel;
    SocketThread* pReaderThread;
    PlayerWidget* pPlayerWidget;
    CheckingPasswdDialog* pCheckingPasswdDialog;

    ChannelWidget* getHostChannel(const QString& channelName);
    Channel* getGuestChannel(const QString& channelName);

signals:
    void connectToServer();
    void disconnectFromServer();
    void sendRequest();

private slots:
    void slotCreateChannel();
    void slotEditChannel();
    void slotHostChannelClicked();
    void slotChooseClicked();
    void slotTurnOffChannel();
    void slotDataReady(QByteArray data);
    void slotConnectionError(QString);
    void slotGuestChannelClicked();
    void slotConnected();
    void slotDisconnected();
    void slotReconnect();
};

#endif // CHOOSECHANNELWIDGET_H
