#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class ChooseChannelWidget;
class QStackedWidget;
class UserNameSettingWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int addWidget(QWidget* widget);
    void setWidget(QWidget* widget);
    void removeWidget(QWidget* widget);
    QString getUserName();

private:
    Ui::MainWindow *ui;
    ChooseChannelWidget* pChannelsWidget;
    UserNameSettingWidget* pUserNameSetting;
    QStackedWidget* pStackOfWidgets;
    QString userName;

private slots:
    void slotUserNameAccepted(const QString& username);

};

#endif // MAINWINDOW_H
