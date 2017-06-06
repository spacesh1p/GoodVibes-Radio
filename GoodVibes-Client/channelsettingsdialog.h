#ifndef CHANNELSETTINGSDIALOG_H
#define CHANNELSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ChannelSettingsDialog;
}

class Channel;

class ChannelSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChannelSettingsDialog(Channel* channel, QWidget *parent = 0);
    ~ChannelSettingsDialog();


private:
    Ui::ChannelSettingsDialog *ui;
    Channel* pChannel;

signals:
    void settingsSeted();

private slots:
    void slotStateChanged();
    void slotAccepted();
};

#endif // CHANNELSETTINGSDIALOG_H
