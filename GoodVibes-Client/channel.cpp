#include <QString>
#include <QDataStream>
#include "channel.h"


Channel::Channel(const QString& host)
    : hostName(host)
{
    // default settings
    channelName = "New channel";
    channelWelcome = "Welcome to " + channelName;
    isPrivate = false;
}

Channel::Channel(const Channel& channel) {
    hostName = channel.hostName;
    channelName = channel.channelName;
    channelWelcome = channel.channelWelcome;
    channelDescription = channel.channelDescription;
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
    out << channel.channelDescription;
    out << channel.isPrivate;
    out << channel.password;
    return out;
}

QDataStream& operator>>(QDataStream& in, Channel& channel) {
    in >> channel.hostName;
    in >> channel.channelName;
    in >> channel.channelWelcome;
    in >> channel.channelDescription;
    in >> channel.isPrivate;
    in >> channel.password;
    return in;
}
