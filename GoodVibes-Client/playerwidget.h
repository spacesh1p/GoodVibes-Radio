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

private slots:
    void slotDataReady(QByteArray data);
    void slotError(const QString& strError);
    void slotConnected();
    void slotDisconnected();
    void slotRestart();
    void slotDisconnectFromChannel();
    void slotMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void slotSetPosition(bool seekable);
    void slotBackClicked();
};

#endif // PLAYERWIDGET_H
