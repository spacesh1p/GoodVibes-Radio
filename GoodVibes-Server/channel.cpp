#include <QString>
#include <QDataStream>
#include "channel.h"

int Channel::newChannelsCounter = 0;

Channel::Channel(const QString& host)
    : hostName(host)
{
    // default settings
    channelName = "New channel";
    if (newChannelsCounter > 0)
        channelName += QString::number(newChannelsCounter);
    channelWelcome = "Welcome to " + channelName;
    maxGuestsNum = 20;
    isPrivate = false;
    newChannelsCounter++;
}

Channel::Channel(const Channel& channel) {
    hostName = channel.hostName;
    channelName = channel.channelName;
    channelWelcome = channel.channelWelcome;
    channelDescription = channel.channelDescription;
    maxGuestsNum = channel.maxGuestsNum;
    isPrivate = channel.isPrivate;
    password = channel.password;
}

void Channel::setHostName(const QString &host) {
    hostName = host;
}

void Channel::setChannelName(const QString& name) {
    channelName = name;
}

void Channel::setChannelWelcome(const QString& welcome) {
    channelWelcome = welcome;
}

void Channel::setChannelDesciption(const QString& descript) {
    channelDescription = descript;
}

void Channel::setMaxGuestsNum(int num) {
    maxGuestsNum = num;
}

void Channel::setPrivate(bool state) {
    isPrivate = state;
}

void Channel::setPassword(const QString& passwd) {
    password = passwd;
}

QString Channel::getHostName() const {
    return hostName;
}

QString Channel::getWelcome() const {
    return channelWelcome;
}

QString Channel::getDescription() const {
    return channelDescription;
}

int Channel::getMaxGuestsNum() const {
    return maxGuestsNum;
}

bool Channel::getPrivateStatus() const {
    return isPrivate;
}

QString Channel::getPassword() const {
    return password;
}

QString Channel::getChannelName() const {
    return channelName;
}


QDataStream& operator<<(QDataStream& out, const Channel& channel) {
    out << channel.hostName;
    out << channel.channelName;
    out << channel.channelWelcome;
    out << channel.maxGuestsNum;
    out << channel.channelDescription;
    out << channel.isPrivate;
    out << channel.password;
    return out;
}

QDataStream& operator>>(QDataStream& in, Channel& channel) {
    in >> channel.hostName;
    in >> channel.channelName;
    in >> channel.channelWelcome;
    in >> channel.maxGuestsNum;
    in >> channel.channelDescription;
    in >> channel.isPrivate;
    in >> channel.password;
    return in;
}
