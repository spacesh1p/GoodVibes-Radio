#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include <QWidget>

namespace Ui {
class ChannelWidget;
}

class Channel;
class ChannelSettingsDialog;
class ChooseChannelWidget;
class MediaHandler;
class QMediaPlayer;
class QCommandLinkButton;
class QFile;
class SocketThread;

class ChannelWidget : public QWidget
{
    Q_OBJECT

public:
    ChannelWidget(Channel* channel, QWidget *parent = 0);
    ~ChannelWidget();
    int openSettingsDialog();
    bool checkPassword();
    Channel* getChannel();
    void addSongButton(QCommandLinkButton* button);
    void removeSongButton(QCommandLinkButton* button);
    void unmute();
    SocketThread* getMediaSender();
    int getSliderValue();

private:
    Ui::ChannelWidget *ui;
    QString userName;
    Channel* pChannel;
    MediaHandler* pMediaHandler;
    QMediaPlayer* pMediaPlayer;
    ChooseChannelWidget* pChooseChannelWidget;
    ChannelSettingsDialog* pSettingsDialog;
    SocketThread* pTextSender;
    SocketThread* pMediaSender;
    int callCnt;

signals:
    void disconnectFromServer();
    void connectToServer();
    void sendChannelData(const Channel& data);
    void sendString(const QString& msg);

private slots:
    void slotOpenFiles();
    void slotBackClicked();
    void slotChannelCreated();
    void slotConnected();
    void slotError(const QString& strError);
    void slotDisconnected();
    void slotRestart();
    void slotFileNotOpened();
    void slotDataReady(QByteArray data);
    void slotUpdateData();
    void slotVolumeChanged(int val);
    void slotVolumeClicked(bool state);
    void slotSeekable(bool state);
    void slotSkip();
    void slotSendMessage();
};


#endif // CHANNELWIDGET_H
