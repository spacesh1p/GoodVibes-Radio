#ifndef CHANNEL_H
#define CHANNEL_H


class QString;
class QDataStream;
class QWidget;

class Channel
{
public:
    Channel();
    Channel(const Channel& channel);
    void setChannelName(const QString& name);
    void setChannelWelcome(const QString& welcome);
    void setChannelDesciption(const QString& descript);
    void setMaxGuestsNum(int num);
    void setPrivate(bool state);
    void setPassword(const QString& passwd);
    QString getWelcome() const;                                                   // get channel welcome
    QString getDescription() const;                                               // get channel description
    int getMaxGuestsNum() const;                                                  // get muximum number of guests
    bool getPrivateStatus() const;                                                // get isPrivate
    QString getPassword() const;                                                  // get channel password
    QString getName() const;                                                      // get channel name

    friend QDataStream &operator<<(QDataStream& out, const Channel& channel);       // over-loaded operators for serialization
    friend QDataStream &operator>>(QDataStream& in, Channel& channel);

private:
    QString channelName;
    QString channelWelcome;
    QString channelDescription;
    int maxGuestsNum;
    bool isPrivate;
    QString password;
    static int newChannelsCounter;                                           // counter of channels
};


#endif // CHANNEL_H
