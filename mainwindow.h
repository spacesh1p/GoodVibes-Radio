#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class ChooseChannelWidget;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //void authentication(QTcpSocket* pSocket);
    int addWidget(QWidget* widget);
    void setWidget(QWidget* widget);
    void removeWidget(QWidget* widget);

private:
    Ui::MainWindow *ui;
    ChooseChannelWidget* pChannelsWidget;
    QStackedWidget* pStackOfWidgets;

};

#endif // MAINWINDOW_H
