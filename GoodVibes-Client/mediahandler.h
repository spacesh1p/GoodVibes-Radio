#ifndef MEDIAHANDLER_H
#define MEDIAHANDLER_H

#include <QObject>
#include <QMediaPlayer>
#include <QQueue>

class QString;
class QCommandLinkButton;
class ChannelWidget;
class SocketThread;

class MediaHandler : public QObject
{
    Q_OBJECT
private:
    QMediaPlayer* pMediaPlayer;
    QQueue<QPair<QString, QCommandLinkButton*>> playList;
    QQueue<QString> filesQueue;
    ChannelWidget* pChannelWidget;
    SocketThread* pMediaSender;
    bool isConnected;

    bool contains(const QString& path);
    QString getFileName(const QString& path);

public:
    MediaHandler(ChannelWidget* channelWidget);
    ~MediaHandler();
    void addSong(const QString& path);
    QMediaPlayer* getMediaPlayer();
    int getPlayListSize();

signals:
    void songAdded(int number);
    void readyToSendFile(const QString& songName, const QString& path);
    void sendString(const QString& msg);

private slots:
    void slotSongAdded(int number);
    void slotMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void slotSendNextSong(QMediaPlayer::MediaStatus status);
    void slotError();
};

#endif // MEDIAHANDLER_H
