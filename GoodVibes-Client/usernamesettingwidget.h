#ifndef USERNAMESETTINGWIDGET_H
#define USERNAMESETTINGWIDGET_H

#include <QWidget>

class SocketThread;

namespace Ui {
class UserNameSettingWidget;
}

class UserNameSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserNameSettingWidget(QWidget *parent = 0);
    ~UserNameSettingWidget();

private:
    Ui::UserNameSettingWidget *ui;
    SocketThread* pSender;
    bool accepted;

signals:
    void usernameAccepted(const QString& username);
    void connectToServer();
    void disconnectFromServer();
    void sendString(const QString& msg);

private slots:
    void slotConnected();
    void slotDataReady(QByteArray);
    void slotDisconnected();
    void slotOkClicked();
    void slotError(const QString& strError);
};

#endif // USERNAMESETTINGWIDGET_H
