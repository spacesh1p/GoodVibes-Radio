#include <QString>
#include <QDataStream>
#include "checkingpasswddialog.h"
#include "channel.h"

int Channel::newChannelsCounter = 0;

Channel::Channel()
{
    // default settings
    channelName = "New channel";
    if (newChannelsCounter > 0)
        channelName += QString::number(newChannelsCounter);
    channelWelcome = "Welcome to " + channelName;
    maxGuestsNum = 20;
    isPrivate = false;
    pPasswdDialog = new CheckingPasswdDialog;
    newChannelsCounter++;
}

Channel::Channel(const Channel& channel) {
    channelName = channel.channelName;
    channelWelcome = channel.channelWelcome;
    channelDescription = channel.channelDescription;
    maxGuestsNum = channel.maxGuestsNum;
    isPrivate = channel.isPrivate;
    password = channel.password;
    pPasswdDialog = new CheckingPasswdDialog;
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

QString Channel::getName() const {
    return channelName;
}


QDataStream& operator<<(QDataStream& out, const Channel& channel) {
    out << channel.channelName;
    out << channel.channelWelcome;
    out << channel.maxGuestsNum;
    out << channel.channelDescription;
    out << channel.isPrivate;
    out << channel.password;
    return out;
}

QDataStream& operator>>(QDataStream& in, Channel& channel) {
    in >> channel.channelName;
    in >> channel.channelWelcome;
    in >> channel.maxGuestsNum;
    in >> channel.channelDescription;
    in >> channel.isPrivate;
    in >> channel.password;
    return in;
}

bool Channel::checkPassword(QWidget* parent) {
    if (pPasswdDialog->parentWidget() == 0)
        pPasswdDialog->setParent(parent);

    if (pPasswdDialog->exec()) {                           // open checking passwd dialog window
        if (pPasswdDialog->getPasswdLine() == password)          // check password
            return true;
    }
    return false;
}
