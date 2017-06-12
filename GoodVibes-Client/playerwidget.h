#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <QBuffer>
#include <QQueue>
#include <QMediaPlayer>

namespace Ui {
class PlayerWidget;
}

class Channel;
class ChooseChannelWidget;
class SocketThread;

class PlayerWidget : public QWidget
{
    Q_OBJECT

public:
    PlayerWidget(QWidget *parent = 0);
    ~PlayerWidget();
    void setChannel(Channel* channel);
    bool isEnabled();

private:
    Ui::PlayerWidget *ui;
    QString userName;
    Channel* pChannel;
    ChooseChannelWidget* pChooseChannelWidget;
    SocketThread* pReaderThread;
    QMediaPlayer* pMediaPlayer;
    bool enabled;
    QQueue<QPair<QString, QByteArray*>> songsQueue;
    bool firstSong;
    QBuffer buffer;
    QQueue<quint64> positionQueue;

    void setNextSong();

signals:
    void disconnectFromServer();
    void connectToServer();
    void sendString(const QString& msg);

private slots:
    void slotDataReady(QByteArray data);
    void slotError(const QString& strError);
    void slotConnected();
    void slotDisconnected();
    void slotDisconnectFromChannel();
    void slotMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void slotSetPosition(bool seekable);
    void slotBackClicked();
    void slotVolumeClicked(bool state);
    void slotVolumeChanged(int val);
    void slotSendMessage();
};

#endif // PLAYERWIDGET_H
