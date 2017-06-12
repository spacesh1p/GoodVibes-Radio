#ifndef CHANNEL_H
#define CHANNEL_H


class QString;
class QDataStream;
class QWidget;

class Channel
{
public:
    Channel(const QString& host = "host");
    Channel(const Channel& channel);
    void setHostName(const QString& host);
    void setChannelName(const QString& chName);
    void setChannelWelcome(const QString& welcome);
    void setChannelDesciption(const QString& descript);
    void setPrivate(bool state);
    void setPassword(const QString& passwd);
    QString getHostName() const;                                                  // get host name
    QString getWelcome() const;                                                   // get channel welcome
    QString getDescription() const;                                               // get channel description
    bool getPrivateStatus() const;                                                // get isPrivate
    QString getPassword() const;                                                  // get channel password
    QString getChannelName() const;                                               // get channel name

    friend QDataStream &operator<<(QDataStream& out, const Channel& channel);       // over-loaded operators for serialization
    friend QDataStream &operator>>(QDataStream& in, Channel& channel);

private:
    QString hostName;
    QString channelName;
    QString channelWelcome;
    QString channelDescription;
    bool isPrivate;
    QString password;
};


#endif // CHANNEL_H
